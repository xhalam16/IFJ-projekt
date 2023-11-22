
#include "header_files/parser.h"
#include "header_files/code_gen.h"
#include "header_files/stack.h"

FILE *f = NULL;
int labelId = 0;

void setGlobalVars()
{
    if (f != NULL)
    {
        return;
    }
    f = fopen("out.ifjcode", "w");

    if (f == NULL)
    {
        return;
    }

    fprintf(f, ".IFJcode23\n");
}

void generateFuncCall(TreeNode *node, bool local)
{
    setGlobalVars();

    fprintf(f, "CREATEFRAME\n");

    if (node->children[1]->children[0]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[1]->numChildren; i++)
        {
            TreeNode *paramValue;
            if (node->children[1]->children[i]->numChildren == 1)
            {
                paramValue = node->children[1]->children[i]->children[0];
            }
            else
            {
                paramValue = node->children[1]->children[i]->children[1];
            }
            fprintf(f, "DEFVAR TF@%%%d\n", i);
            switch (paramValue->type)
            {
            case NODE_INT:
                fprintf(f, "MOVE TF@%%%d int@%d\n", i, node->children[1]->children[i]->token_value.int_value);
                break;
            case NODE_DOUBLE:
                fprintf(f, "MOVE TF@%%%d float@%a\n", i, node->children[1]->children[i]->token_value.double_value);
                break;
            case NODE_STRING:
                fprintf(f, "MOVE TF@%%%d string@%s\n", i, node->children[1]->children[i]->token_value.string_value->buffer);
                break;
            case NODE_IDENTIFIER:
                fprintf(f, "MOVE TF@%%%d LF@%s\n", i, paramValue->label);
                break;
            default:
                break;
            }
        }
    }

    fprintf(f, "CALL %s\n", node->children[0]->label);

    // if (node->children[3]->type != NODE_EPSILON) {
    //     if (local) {
    //         fprintf(f, "MOVE LF@%s TF@%%retval\n", node->children[2]->label);
    //     } else {
    //         fprintf(f, "MOVE GF@%s TF@%%retval\n", node->children[2]->label);
    //     }
    // }
}

void generateCommand(TreeNode *node)
{
    switch (node->type)
    {
    case NODE_FUNCTION_CALL:
        generateFuncCall(node, true);
        break;
    case NODE_RETURN:
        generateReturn(node->children[0]);
        break;
    case NODE_ASSIGN:
        generateAssign(node, true);
        break;
    case NODE_IF_STATEMENT:
        generateIf(node, true);
        break;
    case NODE_WHILE:
        generateWhile(node, true);
        break;
    default:
        break;
    }
}

void generateFuncDeclaration(TreeNode *node, bool local)
{
    setGlobalVars();

    fprintf(f, "JUMP $end$%s\n", node->children[1]->label);
    fprintf(f, "LABEL %s\n", node->children[1]->label);
    fprintf(f, "PUSHFRAME\n");

    if (node->children[3]->type != NODE_EPSILON) /**/
        fprintf(f, "DEFVAR LF@%%retval\n");

    // if (node->children[2]->children[0]->type != NODE_EPSILON)
    // {
    //     for (unsigned i = 0; i < node->children[2]->numChildren; i++)
    //     {
    //         fprintf(f, "DEFVAR LF@param%d\n", i);
    //         fprintf(f, "MOVE LF@%s LF@%%%d\n", node->children[2]->children[i]->label, i);
    //     }
    // }

    /* Pokud má funkce tělo */
    if (node->children[4]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[4]->numChildren; i++) // procházíme příkazy
        {
            generateCommand(node->children[4]->children[i]); // generujeme příkaz
        }
    }

    fprintf(f, "RÍŠA SMRDÍ!!\n");

    fprintf(f, "POPFRAME\n");                                // odstraníme rámec
    fprintf(f, "RETURN\n");                                  // vrátíme se z funkce
    fprintf(f, "LABEL $end$%s\n", node->children[1]->label); // označíme konec funkce
}

void generateReturn(TreeNode *node)
{
    setGlobalVars();

    if (node->children[0]->type != NODE_EPSILON)
    {
        fprintf(f, "MOVE LF@%%retval LF@%s\n", node->children[0]->label);
    }

    // případně generateExpression()
}

void generateAssign(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";
    // fprintf(file, "MOVE %s@%s %s\n", frame, node->children[0]->label, node->children[1]->label);
}

void generateExpression(TreeNode *node, bool local)
{
    setGlobalVars();
}

void generateIf(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res%d\n", frame, labelId);

    // vyhodnocení výrazu

    fprintf(f, "JUMPIFNEQ $if$%d %s@res%d bool@true\n", labelId, frame, labelId);

    if (node->children[1]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[1]->numChildren; i++)
        {
            generateCommand(node->children[1]->children[i]);
        }
    }

    fprintf(f, "JUMP $end$else$%d\n", labelId);
    fprintf(f, "LABEL $else$%d\n", labelId);

    if (node->children[2]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            generateCommand(node->children[2]->children[i]);
        }
    }

    fprintf(f, "LABEL $end$else$%d\n", labelId);

    labelId++;
}

void generateWhile(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res%d\n", frame, labelId);

    for (unsigned i = 0; i < node->numChildren; i++)
    {
        generateCommand(node->children[1]->children[i]);
    }
}

void generateLabel(char *label)
{
    setGlobalVars();

    fprintf(f, "LABEL %s\n", label);
}

void generateDeclaration(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    if (node->type == NODE_DECLARATION)
    {
        fprintf(f, "DEFVAR %s@%s\n", frame, node->children[0]->label);
    }
    else
    {
        fprintf(f, "DEFVAR %s@%s\n", frame, node->children[0]->children[0]->label);

        if (node->children[1]->type == NODE_FUNCTION_CALL)
        {
            generateFuncCall(node->children[1], local);
        }
        else
        {
            generateExpression(node->children[1], local);
        }
    }

    // fprintf(f, "DEFVAR GF@%s\n", node->children[0]->label);
}
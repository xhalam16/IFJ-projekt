
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

void generateFuncCall(TreeNode *node)
{
    setGlobalVars();

    fprintf(f, "CREATEFRAME\n");

    if (node->children[1]->children[0]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[1]->numChildren; i++)
        {
            fprintf(f, "DEFVAR TF@%%%d\n", i);
            fprintf(f, "MOVE TF@%%%d %s\n", i, "int@10");
        }
    }

    fprintf(f, "CALL %s\n", node->children[0]->label);
}

void generateCommand(TreeNode *node) {
    switch (node->type)
    {
    case NODE_FUNCTION_CALL:
        generateFuncCall(node);
        break;
    case NODE_RETURN:
        generateReturn(node->children[0]);
        break;
    case NODE_ASSIGN:
        generateAssign(node, false, false);
        break;
    case NODE_IF_STATEMENT:
        generateIf(node, false, false);
        break;
    case NODE_WHILE:
        generateWhile(node, true);
        break;
    default:
        break;
    }
}

void generateFuncDeclaration(TreeNode *node)
{
    setGlobalVars();

    fprintf(f, "LABEL %s\n", node->children[1]->label);
    fprintf(f, "PUSHFRAME\n");

    if (node->children[3]->type != NODE_EPSILON)
        fprintf(f, "DEFVAR LF@%%retval\n");

    if (node->children[2]->children[0]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            fprintf(f, "DEFVAR LF@param%d\n", i);
            fprintf(f, "MOVE LF@param%d LF@%%%d\n", i, i);
        }
    }

    if (node->children[4]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[4]->numChildren; i++)
        {
            generateCommand(node->children[4]->children[i]);
        }
    }

    fprintf(f, "POPFRAME\n");
    fprintf(f, "RETURN\n");
}

void generateReturn(TreeNode *node)
{
    setGlobalVars();

    if (node->children[0]->type != NODE_EPSILON)
    {
        fprintf(f, "MOVE LF@%%retval LF@%s\n", node->children[0]->label);
    }
}

void generateAssign(TreeNode *node, bool declaration, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    if (declaration)
    {
        fprintf(f, "DEFVAR %s@%s\n", frame, node->children[0]->label);
    }

    // fprintf(file, "MOVE %s@%s %s\n", frame, node->children[0]->label, node->children[1]->label);
}

void generateExpression(TreeNode *node, bool local)
{
    setGlobalVars();

    
}

void generateIf(TreeNode *node, bool local, bool ifElse)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res%d\n", frame, labelId);

    // vyhodnocení výrazu

    fprintf(f, "JUMPIFNEQ $if$%d %s@res%d bool@true\n", labelId, frame, labelId);
    

    for (unsigned i = 0; i < node->numChildren; i++)
    {
        generateCommand(node->children[i]);
    }
    
    fprintf(f, "LABEL $if$%d\n", labelId);

    labelId++;
}

void generateWhile(TreeNode *node, bool local) {
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res%d\n", frame, labelId);

    
}

void generateLabel(char *label)
{
    setGlobalVars();

    fprintf(f, "LABEL %s\n", label);
}


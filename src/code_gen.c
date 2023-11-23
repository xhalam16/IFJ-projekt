
#include "header_files/parser.h"
#include "header_files/code_gen.h"
#include "header_files/stack.h"

FILE *f = NULL;
unsigned labelId = 0;
unsigned retvalId = 0;
unsigned paramId = 0;
unsigned varsId = 0;
int tempResIndex = 0;

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
    case NODE_DECLARATION:
        generateDeclaration(node, true);
    case NODE_EXPRESSION:
        generateExpression(node, true);
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
        fprintf(f, "DEFVAR LF@%%retval%d\n", retvalId++);

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

    fprintf(f, "POPFRAME\n");                                // odstraníme rámec
    fprintf(f, "RETURN\n");                                  // vrátíme se z funkce
    fprintf(f, "LABEL $end$%s\n", node->children[1]->label); // označíme konec funkce
    retvalId--;
}

void generateReturn(TreeNode *node)
{
    setGlobalVars();

    if (node->children[0]->type != NODE_EPSILON)
    {
        fprintf(f, "MOVE LF@%%retval%d LF@%s\n", retvalId, node->children[0]->label);
    }

    // případně generateExpression()
}

void generateAssign(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    if (node->children[0]->type == NODE_DECLARATION)
    {
        generateDeclaration(node->children[0], local);
    }

    generateExpression(node->children[1], local);
    // fprintf(file, "MOVE %s@%s %s\n", frame, node->children[0]->label, node->children[1]->label);
}

/* Pomocná funkce, která rozezná binární operaci a vrací string odpovídající instrukci dané operace */
char *recognize_bin_operation(TreeNode *node)
{
    switch (node->type)
    {
    case NODE_OPERATOR_ADD:
        return "ADD";
    case NODE_OPERATOR_SUB:
        return "SUB";
    case NODE_OPERATOR_MUL:
        return "MUL";
    case NODE_OPERATOR_DIV:
        return "DIV";
    case NODE_OPERATOR_BELOW:
        return "LT";
    case NODE_OPERATOR_ABOVE:
        return "GT";
    case NODE_OPERATOR_BEQ:
        return "LET";
    case NODE_OPERATOR_AEQ:
        return "GET";
    case NODE_OPERATOR_EQUAL:
        return "EQ";
    case NODE_OPERATOR_NEQ:
        return "NEQ";
    case NODE_OPERATOR_NIL_COALESCING:
        return "COAL";
    default:
        break;
    }

    return NULL;
}

/* DŮLEŽITÉ - VELMI DŮLEŽITÉ */
int generateExpression(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    /* Expr = i */
    if (node->numChildren == 1)
    {
        /* Pokud jsme došli k terminálu, vrať pouze index, není potřeba ho zvyšovat kvůli terminálům, protože použijeme label terminálu a vynoř se z rekurze */
        // return tempResIndex;
    }
    /* Expr = expr! */
    else if (node->numChildren == 2)
    {
        return generateExpression(node->children[0], local);
        // fprintf(f, "")

    }
    /* Expr = (expr) nebo Expr = expr op expr */
    else if (node->numChildren == 3)
    {
        /* Expr = (expr) */
        if (node->children[0]->type == NODE_LEFT_PARENTHESIS)
        {
            printf("meeesiiii\n\n\n\n\n\n\n\n\n\n");
            /* Zpracuj expression, který je na indexu 1 a nakonec vrať poslední použitoui hodnotu indexu id pomocných proměnných */
            return generateExpression(node->children[1], local);
        }
        /* Předpokládáme, že první a třetí děti jsou operandy */
        else {
            char *operation = recognize_bin_operation(node->children[1]);
            
            /* Rekurzivně zpracuj nejdříve levý podtrom výrazu a poté pravý podstrom výrazu */
            int a = generateExpression(node->children[0], local);
            int b = generateExpression(node->children[2], local);

            /* Zvyš index counteru pro identifikátory pomocných proměnných pro mezivýsledky */
            tempResIndex++;

            /* Do left_child a right_child ulož hodnoty labelů synů synů pro případ, že půjde o terminály. V tom případě půjde o labely proměnných */
            char *left_child = node->children[0]->children[0]->label;
            char *right_child = node->children[2]->children[0]->label;

            /* Pokud není dítě terminál a nebo je to závorka, nahraď left_child, případně right_child pomocnou proměnnou pro mezivýsledek se správným indexem */
            if (!node->children[0]->children[0]->terminal || node->children[0]->children[0]->type == NODE_LEFT_PARENTHESIS)
            {
                left_child = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                if (left_child == NULL) // Kontrola alokace paměti
                {
                    return -1;
                }
                snprintf(left_child, MAX_VAR_NAME_LENGTH, "$res%d", a); // Převeď návratovou hodnotu a do řetězce pro pomocnou proměnnou
            }
            if (!node->children[2]->children[0]->terminal || node->children[2]->children[0]->type == NODE_LEFT_PARENTHESIS)
            {
                right_child = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                if (right_child == NULL) // Kontrola alokace paměti
                {
                    return -1;
                }
                snprintf(right_child, MAX_VAR_NAME_LENGTH, "$res%d", b); // Převeď návratovou hodnotu b do řetězce pro pomocnou proměnnou
                }

            fprintf(f, "DEFVAR %s@$res%d\n", frame, tempResIndex);
            fprintf(f, "%s %s@res%d %s@%s %s@%s\n", operation, frame, tempResIndex, frame, left_child, frame, right_child);
        }
    }
    printf("temp: %d\n\n", tempResIndex);
    /* Vrať poslední použitý counter indexu pro id pomocných proměnných */
    return tempResIndex;
}

void generateIf(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res%d\n", frame, labelId);

    // vyhodnocení výrazu

    fprintf(f, "JUMPIFNEQ $else$%d %s@res%d bool@true\n", labelId, frame, labelId);

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

    // vyhodnocení výrazu

    fprintf(f, "JUMPIFNEQ $end$while$%d %s@res%d bool@true\n", labelId, frame, labelId);

    for (unsigned i = 0; i < node->numChildren; i++)
    {
        generateCommand(node->children[1]->children[i]);
    }

    fprintf(f, "LABEL $end$while$%d\n", labelId);
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

    if (node->type == NODE_ASSIGN)
    {
        generateAssign(node, local);
    }
    else
    {
        fprintf(f, "DEFVAR %s@%s$%d\n", frame, node->children[0]->label, varsId++);
    }

    // fprintf(f, "DEFVAR %s@%s\n", frame, node->children[0]->children[0]->label);

    // if (node->children[1]->type == NODE_FUNCTION_CALL)
    // {
    //     generateFuncCall(node->children[1], local);
    // }
    // else
    // {
    //     generateExpression(node->children[1], local);
    // }

    // fprintf(f, "DEFVAR GF@%s\n", node->children[0]->label);
}
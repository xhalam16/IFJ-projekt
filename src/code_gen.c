
#include "header_files/parser.h"
#include "header_files/code_gen.h"
#include "header_files/stack.h"

TreeNode *is_terminal(TreeNode *node);

char *recognize_type(TreeNode *node, bool local);

FILE *f = NULL;
unsigned labelId = 0;
unsigned retvalId = 0;
unsigned paramId = 0;
unsigned varsId = 0;
int res_index = 0;

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

    char *frame = local ? "LF" : "GF";

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
                fprintf(f, "MOVE TF@%%%d int@%d\n", i, paramValue->token_value.int_value);
                break;
            case NODE_DOUBLE:
                fprintf(f, "MOVE TF@%%%d float@%a\n", i, paramValue->token_value.double_value);
                break;
            case NODE_STRING:
                fprintf(f, "MOVE TF@%%%d string@%s\n", i, paramValue->label);
                break;
            case NODE_IDENTIFIER:
                fprintf(f, "MOVE TF@%%%d %s@%s\n", i, frame, paramValue->label);
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
        if (node->children[0]->type == NODE_DECLARATION)
        {
            generateDeclaration(node, true);
        }
        else
        {
            generateAssign(node, true);
        }
        break;
    case NODE_IF_STATEMENT:
        generateIf(node, true);
        break;
    case NODE_WHILE:
        generateWhile(node, true);
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
        fprintf(f, "DEFVAR LF@%%retval_%d\n", retvalId);

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
}

void generateReturn(TreeNode *node)
{
    setGlobalVars();

    TreeNode *tree = is_terminal(node);

    char *type;
    char *result;

    
    if (node->type != NODE_EPSILON)
    {

        if (tree != NULL)
        {
            type = recognize_type(tree, true);
            result = tree->label;
        }
        else
        {
            generateExpression(node, true);
            result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (result == NULL) // Kontrola alokace paměti
            {
                return;
            }
            sprintf(result, "$res_%d", ++res_index);
            type = "LF";
        }


        fprintf(f, "MOVE LF@%%retval_%d %s@%s\n", retvalId, type, result);
    }

    

    // případně generateExpression()
}

/* Pomocná funkce, která rozezná binární operaci a vrací string odpovídající instrukci dané operace */
int recognize_bin_operation(TreeNode *node, char **operation_string)
{
    switch (node->type)
    {
    case NODE_OPERATOR_ADD:
        *operation_string = "ADD";
        return NODE_OPERATOR_ADD;
    case NODE_OPERATOR_SUB:
        *operation_string = "SUB";
        return NODE_OPERATOR_SUB;
    case NODE_OPERATOR_MUL:
        *operation_string = "MUL";
        return NODE_OPERATOR_MUL;
    case NODE_OPERATOR_DIV:
        *operation_string = "DIV";
        return NODE_OPERATOR_DIV;
    case NODE_OPERATOR_BELOW:
        *operation_string = "LT";
        return NODE_OPERATOR_BELOW;
    case NODE_OPERATOR_ABOVE:
        *operation_string = "GT";
        return NODE_OPERATOR_ABOVE;
    case NODE_OPERATOR_BEQ:
        *operation_string = "LET";
        return NODE_OPERATOR_BEQ;
    case NODE_OPERATOR_AEQ:
        *operation_string = "GET";
        return NODE_OPERATOR_AEQ;
    case NODE_OPERATOR_EQUAL:
        *operation_string = "EQ";
        return NODE_OPERATOR_EQUAL;
    case NODE_OPERATOR_NEQ:
        *operation_string = "NEQ";
        return NODE_OPERATOR_NEQ;
    case NODE_OPERATOR_NIL_COALESCING:
        *operation_string = "COAL";
        return NODE_OPERATOR_NIL_COALESCING;
    default:
        break;
    }

    return 0;
}

char *recognize_type(TreeNode *node, bool local)
{
    /* Pokud není uzel NULL, urči pro terminál, zda se jedná o konstantu, eventuálně o jakou a vrať její typ ve formě stringu*/
    if (node != NULL)
    {
        switch (node->type)
        {
        case NODE_INT:
            node->label = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (node->label == NULL) // Kontrola alokace paměti
            {
                return NULL;
            }
            sprintf(node->label, "%d", node->token_value.int_value);
            return "int";
        case NODE_DOUBLE:
            node->label = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (node->label == NULL) // Kontrola alokace paměti
            {
                return NULL;
            }
            sprintf(node->label, "%a", node->token_value.double_value);
            return "float";
        case NODE_STRING:
            return "string"; // hodnota literálu je už uložena v atributu label díky jiné funkci, takže není potřeba ji přesoubvat
        case NODE_NIL:
            node->label = "nil";
            return "nil";
        default:
            break;
        }
    }

    /* Pokud je parametr node NULL nebo se jedná o terminál typu identifikátor (proměnná) */
    return local ? "LF" : "GF";
}

/* Funkce vrací ukazatel na terminál na terminální uzel, pokud je výraz terminální */
TreeNode *is_terminal(TreeNode *node)
{
    /* Pokud má uzel pouze jedno dítě, jedná se o terminální výraz a vrací své dítě */
    if (node->numChildren == 1)
    {
        return node->children[0];
    }

    /* Pokud má uzel dva potomky, jedná se o výraz typu expr! a kontrolujeme proto, zda je terminál expr, tedy první dítě */
    if (node->numChildren == 2)
    {
        return is_terminal(node->children[0]);
    }

    /* Pokud má uzel tři potomky a zároveň je typu (expr), kontrolujeme, zda je terminál expr */
    if (node->numChildren == 3 && node->children[1]->type == NODE_EXPRESSION)
    {
        return is_terminal(node->children[1]);
    }

    /**-*/ return NULL;
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
        // return res_index;
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
            /* Zpracuj expression, který je na indexu 1 a nakonec vrať poslední použitoui hodnotu indexu id pomocných proměnných */
            return generateExpression(node->children[1], local);
        }
        /* Předpokládáme, že první a třetí děti jsou operandy */
        else
        {
            char *operation = NULL;
            int operation_id = recognize_bin_operation(node->children[1], &operation);

            /* Pokud je operace sčítání, odčítání, násobení nebo dělení */
            if ((operation_id >= NODE_OPERATOR_ADD && operation_id <= NODE_OPERATOR_DIV) || operation_id == NODE_OPERATOR_BELOW || operation_id == NODE_OPERATOR_ABOVE)
            {
                /* Rekurzivně zpracuj nejdříve levý podtrom výrazu a poté pravý podstrom výrazu */
                int a = generateExpression(node->children[0], local);
                int b = generateExpression(node->children[2], local);

                /* Zvyš index counteru pro identifikátory pomocných proměnných pro mezivýsledky */
                res_index++;

                TreeNode *leftTree = is_terminal(node->children[0]);
                TreeNode *rightTree = is_terminal(node->children[2]);

                char *left_child_type = recognize_type(leftTree, local);

                if (left_child_type == NULL)
                {
                    return -1;
                }

                char *right_child_type = recognize_type(rightTree, local);

                if (right_child_type == NULL)
                {
                    return -1;
                }

                /* stringy pro uložení názvu proměnných */
                char *right_child;
                char *left_child;

                /* Pokud je levé dítě neterminál, nastav název proměnné na pomocnou proměnnou $res_index, podle odpovídajícího indexu */
                if (leftTree == NULL)
                {
                    left_child = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH); // -5 bodov!!!!!!!
                    if (left_child == NULL)                                  // Kontrola alokace paměti
                    {
                        return -1;
                    }
                    snprintf(left_child, MAX_VAR_NAME_LENGTH, "$res_%d", a); // Převeď návratovou hodnotu a do řetězce pro pomocnou proměnnou
                }
                else
                { /* Pokud je pravé dítě terminál, nastav název proměnné na label */
                    left_child = leftTree->label;
                }
                /* Pokud je pravé dítě neterminál, nastav název proměnné na pomocnou proměnnou $res_index, podle odpovídajícího indexu */
                if (rightTree == NULL)
                {
                    printf("RIGHT CHILD TYPE: %d\n", node->children[2]->children[0]->terminal);
                    right_child = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                    if (right_child == NULL) // Kontrola alokace paměti
                    {
                        return -1;
                    }
                    snprintf(right_child, MAX_VAR_NAME_LENGTH, "$res_%d", b); // Převeď návratovou hodnotu b do řetězce pro pomocnou proměnnou
                }
                else /* Pokud je levé dítě terminál, nastav název proměnné na label */
                {
                    right_child = rightTree->label;
                }

                fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
                fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child, left_child_type, right_child);
            }
            else if (operation_id == 5)
            {
            }
        }
    }
    /* Vrať poslední použitý indexu pro id pomocných proměnných */
    return res_index;
}

void generateIf(TreeNode *node, bool local)
{
    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    fprintf(f, "DEFVAR %s@%%res_%d\n", frame, labelId);

    int res = generateExpression(node->children[0], local);

    fprintf(f, "JUMPIFNEQ $else$%d %s@res_%d bool@true\n", labelId, frame, res);

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

    fprintf(f, "DEFVAR %s@%%res_%d\n", frame, labelId);

    int resId = generateExpression(node->children[0], local);

    fprintf(f, "JUMPIFNEQ $end$while$%d %s@res_%d bool@true\n", labelId, frame, resId);

    for (unsigned i = 0; i < node->children[1]->numChildren; i++)
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
    char *label = (node->type == NODE_ASSIGN) ? node->children[0]->children[0]->label : node->children[0]->label;
    fprintf(f, "DEFVAR %s@%s\n", frame, label);

    if (node->type == NODE_ASSIGN)
        generateAssign(node, local);
}

void generateAssign(TreeNode *node, bool local)
{

    setGlobalVars();

    char *frame = local ? "LF" : "GF";

    char *type;
    char *result;

    if (node->children[1]->type == NODE_FUNCTION_CALL)
    {
        generateFuncCall(node->children[1], local);
        result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
        if (result == NULL) // Kontrola alokace paměti
        {
            return;
        }
        sprintf(result, "$retval_%d", retvalId--);
        type = "TF";
    }
    else
    {
        TreeNode *tree = is_terminal(node->children[1]);

        if (tree != NULL)
        {
            type = recognize_type(tree, local);
            result = tree->label;
        }
        else
        {
            generateExpression(node->children[1], local);
            result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (result == NULL) // Kontrola alokace paměti
            {
                return;
            }
            sprintf(result, "$res_%d", res_index);
            type = frame;
        }
    }

    if (node->children[0]->type == NODE_DECLARATION)
    {
        fprintf(f, "MOVE %s@%s %s@%s\n", frame, node->children[0]->children[0]->label, type, result);
    }
    else
    {
        fprintf(f, "MOVE %s@%s %s@%s\n", frame, node->children[0]->label, type, result);
    }
}
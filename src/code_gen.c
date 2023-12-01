
/*
 * Projekt: Překladač jazyka IFJ23
 * Soubor: code_gen.c
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 *        Richard Juřica, xjuric31
 */

#include "header_files/code_gen.h"
#include "header_files/semantic.h"
#include "header_files/stack.h"
#include "header_files/dynamic_buffer.h"

TreeNode *is_terminal(TreeNode *node);

char *recognize_type(TreeNode *node, bool local);

void check_local_tables(char *identifier, bool local);

bool convert_string(char *string);

static FILE *f = NULL;
static unsigned labelId = 0;
static unsigned retvalId = 0;
static unsigned varsId = 0;
static int res_index = 0;
static bool localFunc = false;
static unsigned counter = 0; // počítadlo zanoření
static Stack *local_tables_stack = NULL;
static Stack *varsId_stack = NULL;

bool setGlobalVars(void)
{
    if (f != NULL)
    {
        return true;
    }
    // f = stdout;fopen(stdout, "w");
    f = fopen("out.ifjcode", "w");

    if (f == NULL)
    {
        return false;
    }

    if (local_tables_stack == NULL)
    {
        local_tables_stack = stack_init(STACK_INIT_CAPACITY);
        if (local_tables_stack == NULL)
        {
            fclose(f);
            return false;
        }
    }

    if (varsId_stack == NULL)
    {
        varsId_stack = stack_init(STACK_INIT_CAPACITY);
        if (varsId_stack == NULL)
        {
            stack_free(local_tables_stack);
            fclose(f);
            return false;
        }
    }

    fprintf(f, ".IFJcode23\n");
    return true;
}

void generateFuncCall(TreeNode *node, bool local)
{
    if (!setGlobalVars())
    {
        return;
    }

    fprintf(f, "CREATEFRAME\n");

    char *frame = localFunc ? "LF" : "GF";

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
                printf("STRING1: %s\n", paramValue->label);
                convert_string(paramValue->label);
                printf("STRING2: %s\n", paramValue->label);
                fprintf(f, "MOVE TF@%%%d string@%s\n", i, paramValue->label);
                
                break;
            case NODE_IDENTIFIER:
                check_local_tables(paramValue->label, local);
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
        break;
    case NODE_EXPRESSION:
        generateExpression(node, true);
        break;
    default:
        break;
    }
}

void generateFuncDeclaration(TreeNode *node, bool local)
{
    if (!setGlobalVars())
    {
        return;
    }

    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    arrayInit(local_declarations);

    StackItem *item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    stack_push(local_tables_stack, item);

    fprintf(f, "JUMP $end$%s\n", node->children[1]->label);
    fprintf(f, "LABEL %s\n", node->children[1]->label);
    fprintf(f, "PUSHFRAME\n");

    if (node->children[3]->type != NODE_EPSILON) /**/
        fprintf(f, "DEFVAR LF@%%retval\n");

    if (node->children[2]->children[0]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            fprintf(f, "DEFVAR LF@param_%d\n", i);
            fprintf(f, "MOVE LF@param_%d LF@%%%d\n", i, i);
        }
    }

    localFunc = true;

    counter++;

    /* Pokud má funkce tělo */
    if (node->children[4]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[4]->numChildren; i++) // procházíme příkazy
        {

            generateCommand(node->children[4]->children[i]); // generujeme příkaz
        }
    }

    counter--;

    arrayDispose(local_declarations);
    free(local_declarations);
    free(item);
    stack_pop(local_tables_stack);

    localFunc = false;

    fprintf(f, "POPFRAME\n");                                // odstraníme rámec
    fprintf(f, "RETURN\n");                                  // vrátíme se z funkce
    fprintf(f, "LABEL $end$%s\n", node->children[1]->label); // označíme konec funkce
}

void generateReturn(TreeNode *node)
{
    if (!setGlobalVars())
    {
        return;
    }

    TreeNode *tree = is_terminal(node);

    char *type = localFunc ? "LF" : "GF";
    char *result;

    if (node->children[0]->type == NODE_EPSILON)
    {
        fprintf(f, "RETURN\n");
        return;
    }

    if (node->type == NODE_FUNCTION_CALL)
    {
        generateFuncCall(node, true);
        type = "TF";
        result = "%retval";
    }
    else
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
            sprintf(result, "$res_%d", res_index);
        }
        // generateExpression(node, true);
        // result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
        // if (result == NULL) // Kontrola alokace paměti
        // {
        //     return;
        // }
        // sprintf(result, "$res_%d", ++res_index);
    }

    fprintf(f, "MOVE LF@%%retval %s@%s\n", type, result);
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

void check_local_tables(char *identifier, bool local)
{
    if (stack_size(local_tables_stack) > 0)
    {
        for (int i = stack_size(local_tables_stack) - 1; i > -1; i--)
        {

            for (unsigned j = 0; j < arraySize(((StackItem *)stack_get(local_tables_stack, i)->data)->array); j++)
            {
                if (strcmp(identifier, (((DynamicArray *)((StackItem *)(stack_get(local_tables_stack, i)->data))->array)->items[j].data)) == 0)
                {
                    char *newLabel = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                    sprintf(newLabel, "%s$%d", identifier, ((StackItem *)stack_get(local_tables_stack, i)->data)->index);
                    strcpy(identifier, newLabel);
                }
            }
        }
    }
}

// unsigned hex_to_decimal(const char* hex_string)
// {
//     unsigned result = 0;
//     for (unsigned i = 0; hex_string[i] != '\0'; i++)
//     {
//         result = result * 16 + hex_string[i];
//     }
//     return result;

// }

// kontrola a převod escape sekvencí
bool convert_string(char *string)
{
    DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));

    if (init_buffer(buffer, BUFFER_INIT_CAPACITY) != ERR_CODE_OK) // Kontrola alokace paměti
    {
        return false;
    }

    for (unsigned i = 0; string[i] != '\0'; i++)
    {
        if ((string[i] >= 0 && string[i] <= 32) || string[i] == 35 || string[i] == 92)
        {

            char escape[5];
            if (string[i] < 10) {
                sprintf(escape, "\\00%d", string[i]);
            } else {
                sprintf(escape, "\\0%d", string[i]);
            }
            
            if (buffer_append_string(buffer, escape) != ERR_CODE_OK) // Kontrola alokace paměti
            {
                return false;
            }
            continue;
        }
        
        if (buffer_append_char(buffer, string[i]) != ERR_CODE_OK) // Kontrola alokace paměti
        {
            return false;
        }
    }
    
    move_buffer(&string, buffer);

    return true;
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
            convert_string(node->label);
            return "string"; // hodnota literálu je už uložena v atributu label díky jiné funkci, takže není potřeba ji přesoubvat
        case NODE_NIL:
            node->label = "nil";
            return "nil";
        case NODE_IDENTIFIER:;

            check_local_tables(node->label, local);
            break;
        default:
            break;
        }
    }

    /* Pokud je parametr node NULL nebo se jedná o terminál typu identifikátor (proměnná) */
    return localFunc ? "LF" : "GF";
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

    if (!setGlobalVars())
    {
        return -1;
    }

    char *frame = localFunc ? "LF" : "GF";

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
            /* Zpracuj expression, který je na indexu 1 a nakonec vrať poslední použitou hodnotu indexu id pomocných proměnných */
            return generateExpression(node->children[1], local);
        }
        /* Předpokládáme, že první a třetí děti jsou operandy */
        else
        {

            char *operation = NULL;
            int operation_id = recognize_bin_operation(node->children[1], &operation);

            /* Pokud je operace sčítání, odčítání, násobení dělení, menší, větší nebo rovno */
            if ((operation_id >= NODE_OPERATOR_ADD && operation_id <= NODE_OPERATOR_DIV) || (operation_id >= NODE_OPERATOR_BELOW && operation_id <= NODE_OPERATOR_NEQ))
            {
                /* Rekurzivně zpracuj nejdříve levý podtrom výrazu a poté pravý podstrom výrazu */
                int left_index = generateExpression(node->children[0], local);
                int right_index = generateExpression(node->children[2], local);

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
                char *right_child_varname;
                char *left_child_varname;

                /* Pokud je levé dítě neterminál, nastav název proměnné na pomocnou proměnnou $res_index, podle odpovídajícího indexu */
                if (leftTree == NULL)
                {
                    left_child_varname = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH); // -5 bodov!!!!!!!
                    if (left_child_varname == NULL)                                  // Kontrola alokace paměti
                    {
                        return -1;
                    }
                    snprintf(left_child_varname, MAX_VAR_NAME_LENGTH, "$res_%d", left_index); // Převeď návratovou hodnotu a do řetězce pro pomocnou proměnnou
                }
                else
                { /* Pokud je pravé dítě terminál, nastav název proměnné na label */
                    left_child_varname = leftTree->label;
                }
                /* Pokud je pravé dítě neterminál, nastav název proměnné na pomocnou proměnnou $res_index, podle odpovídajícího indexu */
                if (rightTree == NULL)
                {
                    // printf("RIGHT CHILD TYPE: %d\n", node->children[2]->children[0]->terminal);
                    right_child_varname = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                    if (right_child_varname == NULL) // Kontrola alokace paměti
                    {
                        return -1;
                    }
                    snprintf(right_child_varname, MAX_VAR_NAME_LENGTH, "$res_%d", right_index); // Převeď návratovou hodnotu b do řetězce pro pomocnou proměnnou
                }
                else /* Pokud je levé dítě terminál, nastav název proměnné na label */
                {
                    right_child_varname = rightTree->label;
                }

                fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);

                if (operation_id == NODE_OPERATOR_NEQ)
                {
                    fprintf(f, "EQ %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                    res_index++;

                    fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
                    fprintf(f, "NOT %s@$res_%d %s@$res_%d\n", frame, res_index, frame, res_index - 1); // teoreticky by to mozna slo ulozit do stejne promenne idk
                }
                else if (operation_id == NODE_OPERATOR_AEQ || operation_id == NODE_OPERATOR_BEQ)
                {
                    printf("MESSI\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
                    if (operation_id == NODE_OPERATOR_AEQ)
                    {
                        operation = "LT";
                    }
                    else
                    {
                        operation = "GT";
                    }
                    fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                    res_index++;

                    fprintf(f, "NOT %s@$res_%d %s@res_%d\n", frame, res_index, frame, res_index - 1);
                }
                else
                { // pokud jde o jinou operaci
                    fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                }
            }
        }
    }

    /* Vrať poslední použitý indexu pro id pomocných proměnných */
    return res_index;
}

void generateIf(TreeNode *node, bool local)
{
    if (!setGlobalVars())
    {
        return;
    }

    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    arrayInit(local_declarations);

    StackItem *item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    stack_push(local_tables_stack, item);

    char *frame = localFunc ? "LF" : "GF";

    // fprintf(f, "DEFVAR %s@%%res_%d\n", frame, labelId);

    // int res = generateExpression(node->children[0], local);          Přijde mi to takhle zvláštní

    // fprintf(f, "JUMPIFNEQ $else$%d %s@res_%d bool@true\n", labelId, frame, res);

    generateExpression(node->children[0], local);

    unsigned ifId = labelId++;
    fprintf(f, "JUMPIFNEQ $else$%d %s@res_%d bool@true\n", ifId, frame, res_index);

    counter++;

    if (node->children[1]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[1]->numChildren; i++)
        {
            generateCommand(node->children[1]->children[i]);
        }
    }

    free(local_declarations->items);
    free(local_declarations);
    stack_pop(local_tables_stack);

    fprintf(f, "JUMP $end$else$%d\n", ifId);
    fprintf(f, "LABEL $else$%d\n", ifId);

    local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    arrayInit(local_declarations);

    item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    stack_push(local_tables_stack, item);

    if (node->children[2]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            generateCommand(node->children[2]->children[i]);
        }
    }

    counter--;

    fprintf(f, "LABEL $end$else$%d\n", ifId);

    free(local_declarations->items);
    free(local_declarations);
    stack_pop(local_tables_stack);
}

void generateWhile(TreeNode *node, bool local)
{

    if (!setGlobalVars())
    {
        return;
    }

    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    arrayInit(local_declarations);

    StackItem *item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    stack_push(local_tables_stack, item);

    char *frame = localFunc ? "LF" : "GF";

    int resId = generateExpression(node->children[0], local);

    unsigned endWhile = labelId++;
    fprintf(f, "JUMPIFNEQ $end$while$%d %s@$res_%d bool@true\n", endWhile, frame, resId);

    counter++;

    for (unsigned i = 0; i < node->children[1]->numChildren; i++)
    {
        generateCommand(node->children[1]->children[i]);
    }

    counter--;

    free(stack_top(local_tables_stack)->data);
    stack_pop(local_tables_stack);

    fprintf(f, "LABEL $end$while$%d\n", endWhile);
}

void generateDeclaration(TreeNode *node, bool local)
{

    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *label = (node->type == NODE_ASSIGN) ? node->children[0]->children[0]->label : node->children[0]->label;

    if (stack_size(local_tables_stack) > 0)
    {
        arrayInsert(((StackItem *)((stack_top(local_tables_stack)->data)))->array, label);
    }

    if (!inFunction && counter > 0)
    {
        fprintf(f, "DEFVAR %s@%s$%d\n", frame, label, ((StackItem *)((stack_top(local_tables_stack)->data)))->index);
    }
    else
    {
        fprintf(f, "DEFVAR %s@%s\n", frame, label);
    }

    if (node->type == NODE_ASSIGN)
        generateAssign(node, local);
}

void generateAssign(TreeNode *node, bool local)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";

    char *typeLeft = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
    char *typeRight;
    char *result;

    if (node->children[1]->type == NODE_FUNCTION_CALL)
    {
        generateFuncCall(node->children[1], local);
        result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
        if (result == NULL) // Kontrola alokace paměti
        {
            return;
        }
        result = "%retval";
        typeRight = "TF";
    }
    else
    {
        TreeNode *tree = is_terminal(node->children[1]);

        if (tree != NULL)
        {
            typeRight = recognize_type(tree, local);
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
            typeRight = frame;
        }
    }

    if (node->children[0]->type == NODE_DECLARATION)
    {
        strcpy(typeLeft, node->children[0]->children[0]->label);
    }
    else
    {
        strcpy(typeLeft, node->children[0]->label);
    }

    if (!inFunction)
        check_local_tables(typeLeft, local);
    fprintf(f, "MOVE %s@%s %s@%s\n", frame, typeLeft, typeRight, result);
    free(typeLeft);
}

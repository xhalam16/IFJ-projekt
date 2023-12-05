
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

void check_local_tables(char *identifier, bool left_value);

char *convert_string(char *string);

static FILE *f = NULL;
static unsigned labelId = 0;
static unsigned retvalId = 0;
static unsigned varsId = 0;
static int res_index = 0;
static bool localFunc = false;
static unsigned counter = 0; // počítadlo zanoření
static Stack *local_tables_stack = NULL;
static Stack *varsId_stack = NULL;
static unsigned loop_counter_index = 0; 
static unsigned help_var_index = 0;

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

void generateRead(char *type, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";

    check_local_tables(left_value, false);

    if (left_value)
    {
        fprintf(f, "READ %s@%s %s\n", frame, left_value, type);
    }
}

void generateWrite(TreeNode *parameters)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type;

    for (unsigned i = 0; i < parameters->numChildren; i++)
    {
        type = recognize_type(parameters->children[i]->children[0], true);
        fprintf(f, "WRITE %s@%s\n", type, parameters->children[i]->children[0]->label);
    }
}

void generateInt2Double(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *type = recognize_type(paramValue, true);

    check_local_tables(left_value, false);

    if (left_value)
        fprintf(f, "INT2FLOAT %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

void generateDouble2Int(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *type = recognize_type(paramValue, true);

    check_local_tables(left_value, false);

    if (left_value)
        fprintf(f, "FLOAT2INT %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

void generateLength(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *type = recognize_type(paramValue, true);

    check_local_tables(left_value, false);

    if (left_value)
        fprintf(f, "STRLEN %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

void generateOrd(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *type = recognize_type(paramValue, true);

    check_local_tables(left_value, false);

    if (left_value)
    {
        fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
        fprintf(f, "STRLEN %s@$res_%d %s@%s\n", frame, res_index, type, paramValue->label);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d int@0\n", labelId, frame, res_index++);
        fprintf(f, "STRI2INT %s@%s %s@%s int@0\n", frame, left_value, type, paramValue->label);
        fprintf(f, "JUMP $else$end$%d\n", labelId);
        fprintf(f, "LABEL $else$%d\n", labelId);
        fprintf(f, "MOVE %s@%s int@0\n", frame, left_value);
        fprintf(f, "LABEL $else$end$%d\n", labelId++);
    }
}

void generateChr(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *type = recognize_type(paramValue, true);

    check_local_tables(left_value, false);

    if (left_value)
        fprintf(f, "INT2CHAR %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

// func substring(of 𝑠 : String, startingAt 𝑖 : Int, endingBefore 𝑗 : Int) -> String?
void generateSubString(TreeNode *parameters, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";

    // /* Pomoci dalsi  */
    generateLength(parameters->children[0], left_value);
    
    /* ulozime si index na kterem zacneme substring generovat - druhy parametr funkce */
    int start_index = parameters->children[1]->children[0]->token_value.int_value;

    /* ulozime si index na kterem koncime substring generovat - treti parametr funkce */
    int end_index = parameters->children[2]->children[0]->token_value.int_value;

    if(start_index < 0 || end_index < 0 || start_index > end_index)
    {
        
    }

    check_local_tables(left_value, false);

    if (left_value) {
        fprintf(f, "SUBSTR %s@%s %s@%s int@%d int@%d\n", frame, left_value, frame, parameters->children[0]->children[0]->label, start_index, end_index);
        /* jsou ty parametru takto o jedno dite dal??? otazka na simona */
        char *type_start_index = recognize_type(parameters->children[1]->children[0], true);
        char *type_end_index = recognize_type(parameters->children[2]->children[0], true);
        char *type_string_param = recognize_type(parameters->children[0]->children[0], true);

        fprintf(f, "DEFVAR %s@$cnt_%d\n", frame, loop_counter_index);
        fprintf(f, "DEFVAR %s@$tmp_%d\n", frame, help_var_index);

        fprintf(f, "MOVE %s@$cnt_%d %s@%s \n", frame, loop_counter_index, type_start_index, parameters->children[1]->children[0]->label); // do cnt ulozime start index

        fprintf(f, "LABEL $start_loop$%d\n", labelId);
        fprintf(f, "JUMPIFEQ $end_loop$%d %s@$cnt_%d %s@%s\n", labelId, frame, loop_counter_index, type_end_index, parameters->children[2]->children[0]->label);

        fprintf(f, "GETCHAR %s@tmp_%d %s@%s %s@$cnt_%d\n", frame, help_var_index, type_string_param, parameters->children[0]->children[0]->label, frame, loop_counter_index);
        fprintf(f, "CONCAT %s@%s %s@%s %s@tmp_%d\n", frame, left_value, frame, left_value, frame, help_var_index);

        fprintf(f, "ADD %s@$cnt_%d %s@$cnt_%d int@1\n", frame, loop_counter_index, frame, loop_counter_index);

        fprintf(f, "JUMP $start_loop$%d\n", labelId);

        fprintf(f, "LABEL $end_loop$%d\n", labelId);

        loop_counter_index++;
        labelId++;
        help_var_index++;
    }

}

bool is_built_in_function(TreeNode *funcCall, char *left_value)
{
    if (strcmp(funcCall->children[0]->label, "readInt") == 0)
    {
        generateRead("int", left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "readDouble") == 0)
    {

        generateRead("float", left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "readString") == 0)
    {
        generateRead("string", left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "write") == 0)
    {
        generateWrite(funcCall->children[1]);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "Int2Double") == 0)
    {
        generateInt2Double(funcCall->children[1]->children[0]->children[0], left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "Double2Int") == 0)
    {
        generateDouble2Int(funcCall->children[1]->children[0]->children[0], left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "length") == 0)
    {
        generateLength(funcCall->children[1]->children[0]->children[0], left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "substring") == 0)
    {
        generateSubString(funcCall->children[1], left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "ord") == 0)
    {
        generateOrd(funcCall->children[1]->children[0]->children[0], left_value);
        return true;
    }
    else if (strcmp(funcCall->children[0]->label, "chr") == 0)
    {
        generateChr(funcCall->children[1]->children[0]->children[0], left_value);
        return true;
    }
    return false;
}

void generateFuncCall(TreeNode *node, bool local)
{
    if (!setGlobalVars())
    {
        return;
    }

    if (is_built_in_function(node, NULL))
    {
        return;
    }

    fprintf(f, "CREATEFRAME\n");

    char *frame = localFunc ? "LF" : "GF";
    char *type;

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
            type = recognize_type(paramValue, local);
            // switch (paramValue->type)
            // {
            // case NODE_INT:
            //     fprintf(f, "MOVE TF@%%%d int@%d\n", i, paramValue->token_value.int_value);
            //     break;
            // case NODE_DOUBLE:
            //     fprintf(f, "MOVE TF@%%%d float@%a\n", i, paramValue->token_value.double_value);
            //     break;
            // case NODE_STRING:
            //     paramValue->label = convert_string(paramValue->label);
            //     fprintf(f, "MOVE TF@%%%d string@%s\n", i, paramValue->label);

            //     break;
            // case NODE_IDENTIFIER:
            //     check_local_tables(paramValue->label, false);
            //     fprintf(f, "MOVE TF@%%%d %s@%s\n", i, frame, paramValue->label);
            //     break;
            // default:
            //     break;
            // }
            fprintf(f, "MOVE TF@%%%d %s@%s\n", i, type, paramValue->label);
        }
    }

    fprintf(f, "CALL %s\n", node->children[0]->label);
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
        if (is_built_in_function(node, "%retval"))
        {
            return;
        }
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

void check_local_tables(char *identifier, bool left_value)
{
    if (identifier == NULL)
    {
        return;
    }
    if (stack_size(local_tables_stack) > 0)
    {
        for (int i = stack_size(local_tables_stack) - 1; i > -1; i--)
        {

            for (unsigned j = 0; j < arraySize(((StackItem *)stack_get(local_tables_stack, i)->data)->array); j++)
            {
                // printf("LABEL: %s\n", identifier);

                if (strcmp(identifier, ((ArrayData *)(((DynamicArray *)((StackItem *)(stack_get(local_tables_stack, i)->data))->array)->items[j].data))->label) == 0)
                {
                    char *newLabel = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                    bool *defined = &(((ArrayData *)((StackItem *)stack_get(local_tables_stack, i)->data)->array->items[j].data)->defined);

                    if (*defined || left_value)
                    {
                        sprintf(newLabel, "%s$%d", identifier, ((StackItem *)stack_get(local_tables_stack, i)->data)->index);
                        strcpy(identifier, newLabel);
                    }
                    if (left_value)
                    {
                        *defined = true;
                    }
                    free(newLabel);
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
char *convert_string(char *string)
{
    DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));

    if (init_buffer(buffer, BUFFER_INIT_CAPACITY) != ERR_CODE_OK) // Kontrola alokace paměti
    {
        return NULL;
    }

    for (unsigned i = 0; string[i] != '\0'; i++)
    {
        if ((string[i] >= 0 && string[i] <= 32) || string[i] == 35 || string[i] == 92)
        {

            char escape[5];
            if (string[i] < 10)
            {
                sprintf(escape, "\\00%d", string[i]);
            }
            else
            {
                sprintf(escape, "\\0%d", string[i]);
            }

            if (buffer_append_string(buffer, escape) != ERR_CODE_OK) // Kontrola alokace paměti
            {
                return NULL;
            }
            continue;
        }

        if (buffer_append_char(buffer, string[i]) != ERR_CODE_OK) // Kontrola alokace paměti
        {
            return NULL;
        }
    }

    return buffer->buffer;
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
            node->label = convert_string(node->label);
            return "string"; // hodnota literálu je už uložena v atributu label díky jiné funkci, takže není potřeba ji přesoubvat
        case NODE_NIL:
            node->label = "nil";
            return "nil";
        case NODE_IDENTIFIER:;

            check_local_tables(node->label, false);
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

    return NULL;
}

// bool is_double(TreeNode *node)
// {
//     if (node->type == NODE_DOUBLE)
//     {
//         return true;
//     }
//     return false;
// }


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

                TreeNode *leftTree = is_terminal(node->children[0]); // vrací ukazatel na levý potomek, pokud je to terminál, jinak NULL
                TreeNode *rightTree = is_terminal(node->children[2]); // vrací ukazatel na pravý potomek, pokud je to terminál, jinak NULL

                char *left_child_type = recognize_type(leftTree, local); //

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

                /* Zvyš index counteru pro identifikátory pomocných proměnných pro mezivýsledky */
                res_index++;

                fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);

                /* Operace NOT EQUAL */
                if (operation_id == NODE_OPERATOR_NEQ)
                {
                    fprintf(f, "EQ %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                    res_index++;

                    fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
                    fprintf(f, "NOT %s@$res_%d %s@$res_%d\n", frame, res_index, frame, res_index - 1); // teoreticky by to mozna slo ulozit do stejne promenne idk
                }
                /* Konkatenace stringů */
                else if (operation_id == NODE_OPERATOR_ADD && (node->children[0]->type == NODE_STRING || node->children[0]->type == NODE_STRING_NILABLE)) { // nevim, jestli i String?
                    fprintf(f, "CONCAT %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                }
                /* Operátor ?? */
                else if (operation_id == NODE_OPERATOR_NIL_COALESCING) {
                    /* TYPE dynamicky zjistí hodnotu symbolu a do res zapíše string odpovídající jeho typu - (int, bool, float, string nebo nil) */
                    fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);
                    res_index++;

                    fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d %s@$res_%d, string@nil\n", labelId, frame, res_index, frame, res_index - 1);
                    res_index++;

                    /* Větev, pokud má levý operand hodnotu nil - výsledkem je pravá hodnota */
                    fprintf(f, "MOVE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);
                    fprintf(f, "JUMP $else$end$%d\n", labelId);

                    /* vetev, pokud nemá levý operand hodnotu nil - výsledkem je levá hodnota */
                    fprintf(f, "LABEL $else$%d\n", labelId);
                    fprintf(f, "MOVE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);

                    fprintf(f, "LABEL $else$end$%d\n", labelId);
                    labelId++;
                }
                /* Neostré nerovnosti */
                else if (operation_id == NODE_OPERATOR_AEQ || operation_id == NODE_OPERATOR_BEQ)
                {
                    // if (operation_id == NODE_OPERATOR_AEQ)
                    // {
                    //     operation = "LT";
                    // }
                    // else
                    // {
                    //     operation = "GT";
                    // }
                    switch (operation_id) {
                        case NODE_OPERATOR_AEQ:
                            operation = "LT";
                            break;

                        case NODE_OPERATOR_BEQ:
                            operation = "GT";
                            break;
                    }

                    fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                    res_index++;

                    fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
                    fprintf(f, "NOT %s@$res_%d %s@res_%d\n", frame, res_index, frame, res_index - 1);
                }
                else
                { // pokud jde o jinou operaci
                    if (node->children[0]->type == NODE_DATATYPE_INT && node->children[2]->type == NODE_DATATYPE_DOUBLE)
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
    fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d bool@true\n", ifId, frame, res_index);

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

    fprintf(f, "JUMP $else$end$%d\n", ifId);
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

    fprintf(f, "LABEL $else$end$%d\n", ifId);

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

    ArrayData *data = malloc(sizeof(ArrayData));

    if (data == NULL)
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    char *label = (node->type == NODE_ASSIGN) ? node->children[0]->children[0]->label : node->children[0]->label;

    data->label = malloc(sizeof(char) * strlen(label) + 1);
    strcpy(data->label, label);
    data->defined = false;

    if (stack_size(local_tables_stack) > 0)
    {
        arrayInsert(((StackItem *)((stack_top(local_tables_stack)->data)))->array, data);
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

    char *typeRight;
    char *result;
    char *label;

    if (node->children[0]->type == NODE_DECLARATION)
    {
        label = node->children[0]->children[0]->label;
    }
    else
    {
        label = node->children[0]->label;
    }

    if (node->children[1]->type == NODE_FUNCTION_CALL)
    {

        if (is_built_in_function(node->children[1], label))
        {
            return;
        }

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

    if (!inFunction)
        check_local_tables(label, true);

    fprintf(f, "MOVE %s@%s %s@%s\n", frame, label, typeRight, result);
}

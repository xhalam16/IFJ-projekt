
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

char *recognize_type(TreeNode *node);

char *check_local_tables(char *identifier, bool left_value);

char *convert_string(char *string);

static FILE *f = NULL;
static unsigned labelId = 0; // počítadlo labelů
static unsigned varsId = 0;
static unsigned res_index = 0;
static bool localFunc = false;
static unsigned counter = 0; // počítadlo zanoření
static Stack *local_tables_stack = NULL;
static unsigned loop_counter_index = 0; //index pro pocitadlo cyklu ve funkci substring
static unsigned help_var_index = 0; //index pro pomocnou promennou ve funkci substring

/**
 * @brief Inicializuje globální proměnné, pokud ještě nejsou inicializované
 * @return Vrací true, pokud se inicializace povedla, jinak false
 */
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

    fprintf(f, ".IFJcode23\n");
    return true;
}

// func readInt() -> Int? | func readDouble() -> Double? | func readString() -> String?
void generateRead(char *type, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = check_local_tables(left_value, true);

    if (left_value)
    {
        fprintf(f, "READ %s@%s %s\n", frame, left_value, type);
    }
}

// func write( term_1, term_2, ..., term_n)
void generateWrite(TreeNode *parameters)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type;

    if (parameters->children[0]->type == NODE_EPSILON)
    {
        return;
    }

    for (unsigned i = 0; i < parameters->numChildren; i++)
    {
        type = recognize_type(parameters->children[i]->children[0]);
        fprintf(f, "WRITE %s@%s\n", type, parameters->children[i]->children[0]->label);
    }
}

// func Int2Double(_ term : Int) -> Double
void generateInt2Double(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type = recognize_type(paramValue);
    char *frame = check_local_tables(left_value, true);

    if (left_value)
        fprintf(f, "INT2FLOAT %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

// func Double2Int(_ term : Double) -> Int
void generateDouble2Int(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type = recognize_type(paramValue);
    char *frame = check_local_tables(left_value, true);

    if (left_value)
        fprintf(f, "FLOAT2INT %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

// func length(_ s : String) -> Int
void generateLength(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type = recognize_type(paramValue);

    char *frame = check_local_tables(left_value, true);

    if (left_value)
        fprintf(f, "STRLEN %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

// func ord(_ c : String) -> Int
void generateOrd(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type = recognize_type(paramValue);

    char *frame = check_local_tables(left_value, true);

    if (left_value)
    {
        // pomocná proměnná pro uložení délky řetězce
        fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
        fprintf(f, "STRLEN %s@$res_%d %s@%s\n", frame, res_index, type, paramValue->label);
        // pokud je délka řetězce 0, vrátíme 0
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d int@0\n", labelId, frame, res_index++);
        // pokud je délka řetězce větší než 0, vrátíme znak na indexu 0
        fprintf(f, "STRI2INT %s@%s %s@%s int@0\n", frame, left_value, type, paramValue->label);
        fprintf(f, "JUMP $else$end$%d\n", labelId);
        fprintf(f, "LABEL $else$%d\n", labelId);
        // vrátíme 0
        fprintf(f, "MOVE %s@%s int@0\n", frame, left_value);
        fprintf(f, "LABEL $else$end$%d\n", labelId++);
    }
}

// func chr(_ i : Int) -> String
void generateChr(TreeNode *paramValue, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *type = recognize_type(paramValue);
    char *frame = check_local_tables(left_value, true);

    if (left_value)
        fprintf(f, "INT2CHAR %s@%s %s@%s\n", frame, left_value, type, paramValue->label);
}

/**
 * @brief func substring(of 𝑠 : String, startigAt 𝑖 : Int, endingBefore 𝑗 : Int) -> String?
*/
void generateSubString(TreeNode *parameters, char *left_value)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";

    // /* Pomoci dalsi  */
    fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);

    char *len = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
    snprintf(len, MAX_VAR_NAME_LENGTH, "$res_%d", res_index);

    char *frame_left = check_local_tables(left_value, true);

    if (left_value)
    {
        // fprintf(f, "SUBSTR %s@%s %s@%s %s@%d %s@%d\n", frame, left_value, frame, parameters->children[0]->children[0]->label, start_index, end_index);
        /* jsou ty parametru takto o jedno dite dal??? otazka na simona */
        char *type_start_index = recognize_type(parameters->children[1]->children[1]);
        char *type_end_index = recognize_type(parameters->children[2]->children[1]);
        char *type_string_param = recognize_type(parameters->children[0]->children[1]);

        char *start_index = parameters->children[1]->children[1]->label;
        char *end_index = parameters->children[2]->children[1]->label;
        char *string_param = parameters->children[0]->children[1]->label;

        // if i > j then return nil
        fprintf(f, "GT %s@$res_%d %s@%s %s@%s\n", frame, res_index, type_start_index, start_index, type_end_index, end_index);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d bool@true\n", labelId, frame, res_index);

        // if i < 0 then return nil
        fprintf(f, "LT %s@$res_%d %s@%s int@0\n", frame, res_index, type_start_index, start_index);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d bool@true\n", labelId, frame, res_index);

        // if j < 0 then return nil
        fprintf(f, "LT %s@$res_%d %s@%s int@0\n", frame, res_index, type_end_index, end_index);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d bool@true\n", labelId, frame, res_index);

        generateLength(parameters->children[0]->children[1], len);
        fprintf(f, "DEFVAR %s@$res_%d\n", frame, ++res_index);

        // if i >= length(s) then return nil
        fprintf(f, "LT %s@$res_%d %s@%s %s@%s\n", frame, res_index, type_start_index, start_index, frame, len);
        fprintf(f, "NOT %s@$res_%d %s@$res_%d\n", frame, res_index, frame, res_index);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d bool@true\n", labelId, frame, res_index);

        // if j > length(s) then j = length(s)
        fprintf(f, "GT %s@$res_%d %s@%s %s@%s\n", frame, res_index, type_end_index, end_index, frame, len);
        fprintf(f, "JUMPIFEQ $else$%d %s@$res_%d bool@true\n", labelId, frame, res_index);

        fprintf(f, "MOVE %s@%s string@\n", frame, left_value);
        fprintf(f, "DEFVAR %s@$cnt_%d\n", frame, loop_counter_index);
        fprintf(f, "DEFVAR %s@$tmp_%d\n", frame, help_var_index);

        fprintf(f, "MOVE %s@$cnt_%d %s@%s \n", frame, loop_counter_index, type_start_index, start_index); // do cnt ulozime start index

        fprintf(f, "LABEL $start_loop$%d\n", labelId);
        fprintf(f, "JUMPIFEQ $end_loop$%d %s@$cnt_%d %s@%s\n", labelId, frame, loop_counter_index, type_end_index, end_index);

        fprintf(f, "GETCHAR %s@$tmp_%d %s@%s %s@$cnt_%d\n", frame, help_var_index, type_string_param, string_param, frame, loop_counter_index);
        fprintf(f, "CONCAT %s@%s %s@%s %s@$tmp_%d\n", frame, left_value, frame, left_value, frame, help_var_index);

        fprintf(f, "ADD %s@$cnt_%d %s@$cnt_%d int@1\n", frame, loop_counter_index, frame, loop_counter_index);

        fprintf(f, "JUMP $start_loop$%d\n", labelId);

        fprintf(f, "LABEL $end_loop$%d\n", labelId);
        fprintf(f, "JUMP $else_end$%d\n", labelId);
        fprintf(f, "LABEL $else$%d\n", labelId);
        fprintf(f, "MOVE %s@%s nil@nil\n", frame_left, left_value);
        fprintf(f, "LABEL $else_end$%d\n", labelId);

        loop_counter_index++;
        labelId++;
        help_var_index++;
    }
}

/**
 * @brief kontroluje, zda je funkce vestavěná a případně ji generuje
 * @param funcCall - uzel volání funkce
 * @param left_value - název proměnné, do které se bude ukládat výstup funkce
 * @return Vrací true, pokud je funkce vestavěná, jinak false
 */
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

void generateFuncCall(TreeNode *node)
{
    if (!setGlobalVars())
    {
        return;
    }

    if (is_built_in_function(node, NULL))
    {
        return;
    }

    // vytvoření nového dočasného rámce
    fprintf(f, "CREATEFRAME\n");

    char *frame = localFunc ? "LF" : "GF";
    char *type;

    // uložení parametrů do dočasného rámce
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
            type = recognize_type(paramValue);

            fprintf(f, "MOVE TF@%%%d %s@%s\n", i, type, paramValue->label);
        }
    }

    // zavolání funkce
    fprintf(f, "CALL %s\n", node->children[0]->label);
}

/**
 * @brief generuje příkaz uvnitř bloku
 * @param node - uzel příkazu
 */
void generateCommand(TreeNode *node)
{

    switch (node->type)
    {
    case NODE_FUNCTION_CALL:
        generateFuncCall(node);
        break;
    case NODE_RETURN:
        generateReturn(node->children[0]);
        break;
    case NODE_ASSIGN:
        if (node->children[0]->type == NODE_DECLARATION)
        {
            generateDeclaration(node);
        }
        else
        {
            generateAssign(node);
        }
        break;
    case NODE_DECLARATION:
        generateDeclaration(node);
        break;
    case NODE_IF_STATEMENT:
        generateIf(node);
        break;
    case NODE_WHILE:
        generateWhile(node);
        break;
    default:
        break;
    }
}

void generateFuncDeclaration(TreeNode *node)
{
    if (!setGlobalVars())
    {
        return;
    }

    // lokální tabulka pro parametry funkce implementována jako dynamické pole
    DynamicArray *params = malloc(sizeof(DynamicArray));

    if (params == NULL)
    {
        return;
    }

    // lokální tabulka pro lokální proměnné funkce implementována jako dynamické pole
    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    // inicializace lokálních tabulek
    arrayInit(params);
    arrayInit(local_declarations);

    // vytvoření nové položky na zásobníku pro lokální tabulku parametrů
    StackItem *paramArray = malloc(sizeof(StackItem));
    if (paramArray == NULL)
    {
        return;
    }

    // vytvoření nové položky na zásobníku pro lokální tabulku proměnných
    StackItem *item = malloc(sizeof(StackItem));
    if (item == NULL)
    {
        return;
    }

    paramArray->array = params;
    paramArray->index = varsId++; // index proměnných pro přístup k parametrům

    item->array = local_declarations;
    item->index = varsId++; // index proměnných pro přístup k lokálním proměnným uvnitř funkce

    // vložení tabulek na zásobník
    stack_push(local_tables_stack, paramArray);
    stack_push(local_tables_stack, item);

    fprintf(f, "JUMP $end$%s\n", node->children[1]->label); // skok na konec funkce pokud není ještě definovaná
    fprintf(f, "LABEL %s\n", node->children[1]->label);     // pomocné návěští pro přeskočení nedefinované funkce
    fprintf(f, "PUSHFRAME\n");                              // vložení dočasaného rámce na zásobník lokálních rámců

    if (node->children[3]->type != NODE_EPSILON) // pokud má funkce návratovou hodnotu
        fprintf(f, "DEFVAR LF@%%retval\n");

    // definice parametrů funkce a vložení do tabulky symbolů
    if (node->children[2]->children[0]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            TreeNode *param = node->children[2]->children[i];

            ArrayData *data = malloc(sizeof(ArrayItem));
            if (data == NULL)
            {
                return;
            }

            data->label = param->children[1]->label;
            data->defined = true;
            arrayInsert(params, data);

            fprintf(f, "DEFVAR LF@%s$%d\n", param->children[1]->label, varsId - 2);
            fprintf(f, "MOVE LF@%s$%d LF@%%%d\n", param->children[1]->label, varsId - 2, i);
        }
    }

    localFunc = true; // bude se pracovat s lokálním rámcem

    counter++; // zvýšení počítadla zanoření

    /* Pokud má funkce tělo */
    if (node->children[4]->type != NODE_EPSILON)
    {
        for (unsigned i = 0; i < node->children[4]->numChildren; i++) // procházíme příkazy
        {
            generateCommand(node->children[4]->children[i]); // generujeme příkaz
        }
    }

    counter--; // snížení počítadla zanoření

    // uvolnění a popnutí lokálních tabulek ze zásobníku
    arrayDispose(local_declarations);
    free(local_declarations);
    free(item);
    stack_pop(local_tables_stack);
    arrayDispose(params);
    free(params);
    free(paramArray);
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

    char *type;
    char *result;

    if (node->children[0]->type == NODE_EPSILON) // pokud se provádí return bez návratové hodnoty
    {
        fprintf(f, "RETURN\n");
        return;
    }

    if (node->type == NODE_FUNCTION_CALL) // pokud je návratová hodnota volání funkce
    {
        if (is_built_in_function(node, "%retval"))
        {
            return;
        }
        generateFuncCall(node);
        type = "TF";
        result = "%retval";
    }
    else
    {
        if (tree != NULL) // pokud je návratová hodnota jeden operand
        {
            type = recognize_type(tree);
            result = tree->label;
        }
        else // pokud je návratová hodnota výraz
        {
            generateExpression(node);
            result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (result == NULL) // Kontrola alokace paměti
            {
                return;
            }
            sprintf(result, "$res_%d", res_index++);
        }
    }

    // uložení návratové hodnoty
    fprintf(f, "MOVE LF@%%retval %s@%s\n", type, result);
}

/**
 * @brief  * @brief Funkce vrací ukazatel na terminál na terminální uzel, pokud je výraz terminál
*/
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

/**
 * @brief Hledá v zásobníku lokálních tabulek, v jakém bloku s jakým indexem byla proměnná definovaná
 * @param identifier - název hledané proměnné
 * @param left_value - zda se jedná o levou stranu přiřazení
 */
char *check_local_tables(char *identifier, bool left_value)
{
    if (identifier == NULL)
    {
        return NULL;
    }
    char *frame = "GF"; // pokud není nalezna v lokální tabulce, jedná se o globální proměnnou

    if (stack_size(local_tables_stack) > 0) // pokud není zásobník lokálních tabulek prázdný
    {
        for (int i = stack_size(local_tables_stack) - 1; i > -1; i--) // procházíme zásobník od vrcholu
        {

            for (unsigned j = 0; j < arraySize(((StackItem *)stack_get(local_tables_stack, i)->data)->array); j++) // procházíme tabulku symbolů
            {                                                                                                      // pokud je proměnná nalezena v tabulce symbolů
                if (strcmp(identifier, ((ArrayData *)(((DynamicArray *)((StackItem *)(stack_get(local_tables_stack, i)->data))->array)->items[j].data))->label) == 0)
                {
                    char *newLabel = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
                    bool *defined = &(((ArrayData *)((StackItem *)stack_get(local_tables_stack, i)->data)->array->items[j].data)->defined);
                    // pokud je proměnná definovaná nebo je právě teď definována
                    if (*defined || left_value)
                    {
                        frame = "LF";
                        // k názvu proměnné přidáme index bloku, ve kterém byla proměnná definovaná
                        sprintf(newLabel, "%s$%d", identifier, ((StackItem *)stack_get(local_tables_stack, i)->data)->index);
                        strcpy(identifier, newLabel);
                    }
                    if (left_value) // pokud byla proměnná právě definována, nastavíme ji jako definovanou
                    {
                        *defined = true;
                    }
                    free(newLabel);
                }
            }
        }
    }
    return frame;
}

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

/**
 * @brief Funkce vrací string odpovídající typu konstanty, případně LF nebo GF, pokud se jedná o identifikátor
*/
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
            return check_local_tables(node->label, false);
            break;
        default:
            break;
        }
    }

    /* Pokud je parametr node NULL nebo se jedná o terminál typu identifikátor (proměnná) */
    return localFunc ? "LF" : "GF";
}

/**
 * @brief Funkce vrací ukazatel na terminál na terminální uzel, pokud je výraz terminál
*/
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

/**
 * @brief Kontroluje, zda je jeden z operandů typu double a druhý typu int literál. V takovém případě převádí int literál na double
*/
void check_operand_types_literal(TreeNode *node, bool local, char **left_child_type, char *left_child_varname, char **right_child_type, char *right_child_varname) {
    char *frame = localFunc ? "LF" : "GF";

    /* Chceme zkontrolovat, zda pokud je jeden z operandů literál typu Int, zda není druhý operand typu Double. Pokud ano, převedeme int literál na Double */
    if (node->children[0]->children[0]->type == NODE_INT || node->children[2]->children[0]->type == NODE_INT)
    {
        /* Pokud je levý operand literál typu Int */
        if (node->children[0]->children[0]->type == NODE_INT && node->children[2]->children[0]->type != NODE_INT)
        {
            fprintf(f, "DEFVAR %s@$res_%d\n", frame, ++res_index);
            fprintf(f, "MOVE %s@$res_%d %s@%s\n", frame, res_index, *left_child_type, left_child_varname);
            fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index - 1, *right_child_type, right_child_varname);
            fprintf(f, "JUMPIFNEQ $else_lit$%d %s@$res_%d string@float\n", labelId, frame, res_index - 1);

            char *left_value = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            snprintf(left_value, MAX_VAR_NAME_LENGTH, "$res_%d", res_index);

            generateInt2Double(node->children[0]->children[0], left_value);
            *left_child_type = frame;
            strcpy(left_child_varname, left_value);
            free(left_value);

            fprintf(f, "LABEL $else_lit$%d\n", labelId);
        }
        /* Pokud je pravý operand literál typu Int */
        else if (node->children[2]->children[0]->type == NODE_INT && node->children[0]->children[0]->type != NODE_INT)
        {
            fprintf(f, "DEFVAR %s@$res_%d\n", frame, ++res_index);
            fprintf(f, "MOVE %s@$res_%d %s@%s\n", frame, res_index, *right_child_type, right_child_varname);
            fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index - 1, *left_child_type, left_child_varname);
            fprintf(f, "JUMPIFNEQ $else_lit$%d %s@$res_%d string@float\n", labelId, frame, res_index - 1);
            char *left_value = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            snprintf(left_value, MAX_VAR_NAME_LENGTH, "$res_%d", res_index);

            generateInt2Double(node->children[2]->children[0], left_value);
            *right_child_type = frame;
            strcpy(right_child_varname, left_value);
            free(left_value);

            fprintf(f, "LABEL $else_lit$%d\n", labelId);
        }
    }
}

/**
 *  @brief Tato funkce zkontroluje, zda je jeden operátor typu float a druhý typu int. Pokud ano, musí převést operand typu int na typ float.
 *  V takovém případě je totuž operand typu int buď literál, který je třeba převést dne zadání nebo proměnná typu int, která však byla přiřazena v původním jazyce do Double promenne
*/
void check_operand_types_var(TreeNode *node, bool local, char *left_child_type, char *left_child_varname, char *right_child_type, char *right_child_varname) {
    char *frame = localFunc ? "LF" : "GF";

    if (node->children[0]->children[0]->type == NODE_IDENTIFIER && node->children[2]->children[0]->type == NODE_IDENTIFIER)
    {
        fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);
        res_index++;
        fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
        fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, right_child_type, right_child_varname);
        /* Pokud jsou promenne stejneho typu, neni potreba nic konvertovat a skaceme na konec */
        fprintf(f, "JUMPIFEQ $else_var_end$%d %s@$res_%d %s@$res_%d\n", labelId, frame, res_index, frame, res_index - 1);
        /* Pokud není první operand typu int, skáčeme a musíme přetypovat druhý operand */
        fprintf(f, "JUMPIFNEQ $else_var$%d %s@res_%d string@int\n", labelId, frame, res_index - 1);
        /* Přetypujeme první operand */
        fprintf(f, "INT2FLOAT %s@%s %s@%s\n", left_child_type, left_child_varname, left_child_type, left_child_varname);
        fprintf(f, "JUMP $else_var_end$%d\n", labelId); // skaceme na konec
        fprintf(f, "LABEL $else_var$%d\n", labelId);
        /* Přetypujeme druhý operand */
        fprintf(f, "INT2FLOAT %s@%s %s@%s\n", right_child_type, right_child_varname, right_child_type, right_child_varname);
        fprintf(f, "LABEL $else_var_end$%d\n", labelId);
    }
}


/**
 * @brief Funkce generuje instrukce pro výpočet výrazu, který je předán jako parametr
*/
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
    }
    /* Expr = expr! */
    else if (node->numChildren == 2)
    {
        return generateExpression(node->children[0]);
    }
    /* Expr = (expr) nebo Expr = expr op expr */
    else if (node->numChildren == 3)
    {

        /* Expr = (expr) */
        if (node->children[0]->type == NODE_LEFT_PARENTHESIS)
        {
            /* Zpracuj expression, který je na indexu 1 a nakonec vrať poslední použitou hodnotu indexu id pomocných proměnných */
            return generateExpression(node->children[1]);
        }
        /* Předpokládáme, že první a třetí děti jsou operandy - jde o binární operaci */
        else
        {

            char *operation = NULL;
            int operation_id = recognize_bin_operation(node->children[1], &operation);

            /* Rekurzivně zpracuj nejdříve levý podtrom výrazu a poté pravý podstrom výrazu */
            int left_index = generateExpression(node->children[0]);
            int right_index = generateExpression(node->children[2]);

            TreeNode *leftTree = is_terminal(node->children[0]);  // vrací ukazatel na levý potomek, pokud je to terminál, jinak NULL
            TreeNode *rightTree = is_terminal(node->children[2]); // vrací ukazatel na pravý potomek, pokud je to terminál, jinak NULL

            char *left_child_type = recognize_type(leftTree); //

            if (left_child_type == NULL)
            {
                return -1;
            }

            char *right_child_type = recognize_type(rightTree);

            if (right_child_type == NULL)
            {
                return -1;
            }

            /* Zvyš index counteru pro identifikátory pomocných proměnných pro mezivýsledky */
            res_index++;

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

            /* Operace NOT EQUAL */
            if (operation_id == NODE_OPERATOR_NEQ)
            {
                fprintf(f, "EQ %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                fprintf(f, "NOT %s@$res_%d %s@$res_%d\n", frame, res_index, frame, res_index); // teoreticky by to mozna slo ulozit do stejne promenne idk
            }
            /* Operátor ?? */
            else if (operation_id == NODE_OPERATOR_NIL_COALESCING)
            {
                /* TYPE dynamicky zjistí hodnotu symbolu a do res zapíše string odpovídající jeho typu - (int, bool, float, string nebo nil) */
                fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);
                res_index++;
                fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);

                fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d string@nil\n", labelId, frame, res_index - 1);

                /* Větev, pokud má levý operand hodnotu nil - výsledkem je pravá hodnota */
                fprintf(f, "MOVE %s@$res_%d %s@%s\n", frame, res_index, right_child_type, right_child_varname);
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
                switch (operation_id)
                {
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
                fprintf(f, "NOT %s@$res_%d %s@$res_%d\n", frame, res_index, frame, res_index - 1);
            }
            /* Operátor + má speciální význam v tom, že když se jedná o opearndy typu String, provádí se konkatenace */
            else if (operation_id == NODE_OPERATOR_ADD)
            {
                fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);

                fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d string@string\n", labelId, frame, res_index);

                /* Pokud jsou operandy typu string, provádí se konkatenace */
                fprintf(f, "CONCAT %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                fprintf(f, "JUMP $else$end$%d\n", labelId);

                /* Pokud není první operand typu string, urcite je typu int nebo double takze provedeme klasickou aritmetickou operaci */
                fprintf(f, "LABEL $else$%d\n", labelId);

                check_operand_types_literal(node, &left_child_type, left_child_varname, &right_child_type, right_child_varname);
                check_operand_types_var(node, left_child_type, left_child_varname, right_child_type, right_child_varname);

                fprintf(f, "ADD %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);

                fprintf(f, "LABEL $else$end$%d\n", labelId);
                labelId++;
            }
            /* Operátor dělení má dvě speciální instrukce DIV (oba operandy float) nebo IDIV (oba operandy int) */
            else if (operation_id == NODE_OPERATOR_DIV)
            {
                check_operand_types_var(node, left_child_type, left_child_varname, right_child_type, right_child_varname);
                fprintf(f, "TYPE %s@$res_%d %s@%s\n", frame, res_index, left_child_type, left_child_varname);

                fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d string@float\n", labelId, frame, res_index);

                /* Pokud jsou operandy typu float */
                fprintf(f, "DIV %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
                fprintf(f, "JUMP $else$end$%d\n", labelId);

                /* Pokud jsou operandy typu int */
                fprintf(f, "LABEL $else$%d\n", labelId);
                fprintf(f, "IDIV %s@$res_%d %s@%s %s@%s\n", frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);

                fprintf(f, "LABEL $else$end$%d\n", labelId);
                labelId++;
            }
            /* Pokud jde o operace - nebo * */
            else if(operation_id == NODE_OPERATOR_SUB || operation_id == NODE_OPERATOR_MUL) {
                /* U těchto operací zkontroluji, zda není náhodou jedna proměnná int a druhá float, potom bych přetypoval na int na float */
                check_operand_types_var(node, local, left_child_type, left_child_varname, right_child_type, right_child_varname);
                /* Dále zkontrolujeme, zda není jeden z operandů typu float a druhý int literál, potom bychom přetypovali int literál na float */
                check_operand_types_literal(node, local, &left_child_type, left_child_varname, &right_child_type, right_child_varname);

                fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
            }
            else
            { // pokud jde o jinou operaci - tzn. : <, >, ==
                fprintf(f, "%s %s@$res_%d %s@%s %s@%s\n", operation, frame, res_index, left_child_type, left_child_varname, right_child_type, right_child_varname);
            }
        }
    }

    /* Vrať poslední použitý indexu pro id pomocných proměnných */
    return res_index;
}

void generateIf(TreeNode *node)
{
    if (!setGlobalVars())
    {
        return;
    }

    // vytvoření lokální tabulky pro blok ifu
    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    // inicializace lokální tabulky
    arrayInit(local_declarations);

    // vytvoření položky zásobníku
    StackItem *item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    // vložení lokální tabulky na zásobník
    stack_push(local_tables_stack, item);

    char *frame = localFunc ? "LF" : "GF";

    if (node->children[0]->type == NODE_GUARD_LET) // pokud je místo podmínky 'let id'
    {
        char *frame_id = check_local_tables(node->children[0]->children[0]->label, false);
        fprintf(f, "DEFVAR %s@$res_%d\n", frame, res_index);
        fprintf(f, "EQ %s@$res_%d %s@%s nil@nil\n", frame, res_index, frame_id, node->children[0]->children[0]->label);
    }
    else // generování podmínky
    {
        generateExpression(node->children[0]);
    }

    unsigned ifId = labelId++;
    // skok na else v případě, že je podmínka nesplněna
    fprintf(f, "JUMPIFNEQ $else$%d %s@$res_%d bool@true\n", ifId, frame, res_index++);

    counter++;

    // generování příkazů v bloku if
    if (node->children[1]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[1]->numChildren; i++)
        {
            generateCommand(node->children[1]->children[i]);
        }
    }

    // popnutí lokální tabulky bloku if ze zásobníku
    free(local_declarations->items);
    free(local_declarations);
    stack_pop(local_tables_stack);

    // skok za else
    fprintf(f, "JUMP $else$end$%d\n", ifId);
    // začátek else
    fprintf(f, "LABEL $else$%d\n", ifId);

    // vytvoření lokální tabulky pro blok else
    local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    // inicializace lokální tabulky
    arrayInit(local_declarations);

    // vytvoření položky zásobníku
    item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    // vložení lokální tabulky na zásobník
    stack_push(local_tables_stack, item);

    // generování příkazů v bloku else
    if (node->children[2]->children[0]->type != NODE_BODY_END)
    {
        for (unsigned i = 0; i < node->children[2]->numChildren; i++)
        {
            generateCommand(node->children[2]->children[i]);
        }
    }

    counter--;

    // konec if else
    fprintf(f, "LABEL $else$end$%d\n", ifId);

    // popnutí lokální tabulky bloku else ze zásobníku
    free(local_declarations->items);
    free(local_declarations);
    stack_pop(local_tables_stack);
}

void generateWhile(TreeNode *node)
{

    if (!setGlobalVars())
    {
        return;
    }

    // vytvoření lokální tabulky pro blok while
    DynamicArray *local_declarations = malloc(sizeof(DynamicArray));

    if (local_declarations == NULL)
    {
        return;
    }

    // inicializace lokální tabulky
    arrayInit(local_declarations);

    // vytvoření položky zásobníku
    StackItem *item = malloc(sizeof(StackItem));

    if (item == NULL)
    {
        return;
    }

    item->array = local_declarations;
    item->index = varsId++;

    // vložení lokální tabulky na zásobník
    stack_push(local_tables_stack, item);

    char *frame = localFunc ? "LF" : "GF";

    // generování podmínky
    generateExpression(node->children[0]);

    unsigned endWhile = labelId++;
    // skok za while v případě, že je podmínka nesplněna
    fprintf(f, "JUMPIFNEQ $end$while$%d %s@$res_%d bool@true\n", endWhile, frame, res_index++);

    counter++;

    // generování příkazů v bloku while
    for (unsigned i = 0; i < node->children[1]->numChildren; i++)
    {
        generateCommand(node->children[1]->children[i]);
    }

    counter--;

    // popnutí lokální tabulky bloku while ze zásobníku
    free(local_declarations->items);
    free(local_declarations);
    stack_pop(local_tables_stack);

    // konec while bloku
    fprintf(f, "LABEL $end$while$%d\n", endWhile);
}

void generateDeclaration(TreeNode *node)
{

    if (!setGlobalVars())
    {
        return;
    }

    // vytvoření nových dat o deklarované proměnné pro uložení do lokální tabulky
    ArrayData *data = malloc(sizeof(ArrayData));

    if (data == NULL)
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";
    // zda je deklarace součástí přiřazení
    char *label = (node->type == NODE_ASSIGN) ? node->children[0]->children[0]->label : node->children[0]->label;

    data->label = malloc(sizeof(char) * strlen(label) + 1);
    strcpy(data->label, label);
    data->defined = false;

    // pokud na zásobníku je nějaká lokální tabulka
    if (stack_size(local_tables_stack) > 0)
    {
        arrayInsert(((StackItem *)((stack_top(local_tables_stack)->data)))->array, data);
    }

    if (!inFunction && counter > 0) // pokud se jedná o lokální proměnnou
    {
        fprintf(f, "DEFVAR %s@%s$%d\n", frame, label, ((StackItem *)((stack_top(local_tables_stack)->data)))->index);
    }
    else // pokud se jedná o globální proměnnou
    {
        fprintf(f, "DEFVAR %s@%s\n", frame, label);
    }

    // pokud je deklarace součástí přiřazení
    if (node->type == NODE_ASSIGN)
        generateAssign(node);
}

void generateAssign(TreeNode *node)
{
    if (!setGlobalVars())
    {
        return;
    }

    char *frame = localFunc ? "LF" : "GF";

    char *typeRight; // typ výrazu na pravé straně přiřazení
    char *result;   // název proměnné, do které se uloží výsledek výrazu na pravé straně přiřazení
    char *label; // název proměnné, do které se přiřazuje

    // pokud je přiřazení součástí deklarace
    if (node->children[0]->type == NODE_DECLARATION)
    {
        label = node->children[0]->children[0]->label;
    }
    else
    {
        label = node->children[0]->label;
    }

    // pokud je na pravé straně přiřazení volání funkce
    if (node->children[1]->type == NODE_FUNCTION_CALL)
    {
        // kontrola zda se nejedná o vestavěnou funkci
        if (is_built_in_function(node->children[1], label))
        {
            return;
        }

        generateFuncCall(node->children[1]);
        result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
        if (result == NULL) // Kontrola alokace paměti
        {
            return;
        }
        result = "%retval"; // výsledek je návratová hodnota funkce
        typeRight = "TF"; // návratová hodnota je vždy uložena v dočasném rámci
    }
    else
    {
        
        TreeNode *tree = is_terminal(node->children[1]);

        if (tree != NULL) // pokud je na pravé straně přiřazení jeden operand
        {
            typeRight = recognize_type(tree);
            result = tree->label;
        }
        else // generování výrazu na pravé straně přiřazení
        {
            generateExpression(node->children[1]);
            result = malloc(sizeof(char) * MAX_VAR_NAME_LENGTH);
            if (result == NULL) // Kontrola alokace paměti
            {
                return;
            }
            // výsledek je uložen v pomocné proměnné
            sprintf(result, "$res_%d", res_index++);
            typeRight = frame;
        }
    }

    frame = check_local_tables(label, true);

    // operace přiřazení
    fprintf(f, "MOVE %s@%s %s@%s\n", frame, label, typeRight, result);
}
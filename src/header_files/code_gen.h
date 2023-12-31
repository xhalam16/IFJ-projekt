/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: code_gen.h
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 *        Richard Juřica, xjuric31
 */


#include <stdio.h>
#include <stdbool.h>
#include "parser.h"


/**
 * @brief Macro setting the initial capacity of var names
*/
#define MAX_VAR_NAME_LENGTH 75


/**
 * @brief Private global variables for the symbol table
 * @param f The file to write the code to
 * @param labelId counter that ensures unique labels
 * @param varsId counter that ensures unique variable names
 * @param res_index stores temporary result of an expression
 * @param localFunc signalizes, whether we are in a function
 * @param counter counter that counts the number of nested blocks
 * @param local_tables_stack stack of local tables
*/
static FILE *f;
static unsigned labelId;
static unsigned varsId;
static unsigned res_index;
static bool localFunc;
static unsigned counter;
static Stack *local_tables_stack;
static unsigned loop_counter_index;
static unsigned help_var_index;


/**
 * @brief Struct defining data stored in a Stack
 * @param array The array of dynamic arrays
 * @param index The index of variable in a nested block
 * @typedef StackItem
*/
typedef struct StackItem {
    DynamicArray *array;
    unsigned index;
} StackItem;


/**
 * @brief Structure defining data stored in a dynamic array
 * @param label The name of the variable
 * @param defined Whether the variable is defined
 * @typedef ArrayData
*/
typedef struct ArrayData {
    char *label;
    bool defined;
} ArrayData;


/**
 * @brief Function generating code for a function call
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/
void generateFuncCall(TreeNode *node);

/**
 * @brief Function generating code for function declaration
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/

void generateFuncDeclaration(TreeNode *node);


/**
 * @brief Function generating code for a return statement
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/
void generateReturn(TreeNode *node);


/**
 * @brief Function generating code for assign statement
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/

void generateAssign(TreeNode *node);


/**
 * @brief Function generating code for if...else statement
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/
void generateIf(TreeNode *node);

/**
 * @brief Function generating code for while statement
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/

void generateWhile(TreeNode *node);

/**
 * @brief Function generating code for expression
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
 * @return The index of the variable, where the result of the expression is stored
*/

int generateExpression(TreeNode *node);

/**
 * @brief Function generating code for declaration
 * @note Outputs the code to the global variable f (stdout default)
 * @param node The node to generate the code from
*/

void generateDeclaration(TreeNode *node);

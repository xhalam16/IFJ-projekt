/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: symtable.c
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 *        Richard Juřica, xjuric31
 */


#include <stdio.h>
#include <stdbool.h>
#include "parser.h"



#define MAX_VAR_NAME_LENGTH 75

static FILE *f;
static unsigned labelId;
static unsigned varsId;
static int res_index;
static bool localFunc;
static unsigned counter;
static Stack *local_tables_stack;
static unsigned loop_counter_index;
static unsigned help_var_index;

typedef struct StackItem {
    DynamicArray *array;
    unsigned index;
} StackItem;

typedef struct ArrayData {
    char *label;
    bool defined;
} ArrayData;

void generateFuncCall(TreeNode *node, bool local);

void generateFuncDeclaration(TreeNode *node, bool local);

void generateReturn(TreeNode *node);

void generateAssign(TreeNode *node, bool local);

void generateIf(TreeNode *node, bool local);

void generateWhile(TreeNode *node, bool local);

int generateExpression(TreeNode *node, bool local);

void generateDeclaration(TreeNode *node, bool local);

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

extern FILE *f;
extern unsigned labelId;
extern unsigned retvalId;
extern unsigned paramId;
extern unsigned varsId;
extern int res_index;
extern bool localFunc;
extern bool inFunction;
extern unsigned counter;
extern Stack *local_tables_stack;
extern Stack *varsId_stack;

typedef struct StackItem {
    DynamicArray *array;
    unsigned index;
} StackItem;

void generateFuncCall(TreeNode *node, bool local);

void generateFuncDeclaration(TreeNode *node, bool local);

void generateReturn(TreeNode *node);

void generateAssign(TreeNode *node, bool local);

void generateIf(TreeNode *node, bool local);

void generateLabel(char *label);

void generateWhile(TreeNode *node, bool local);

int generateExpression(TreeNode *node, bool local);

void generateDeclaration(TreeNode *node, bool local);

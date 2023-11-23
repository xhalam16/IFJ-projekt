#include <stdio.h>
#include <stdbool.h>

#define MAX_VAR_NAME_LENGTH 75

extern FILE *f;
extern unsigned labelId;
extern unsigned retvalId;
extern unsigned paramId;
extern unsigned varsId;

void generateFuncCall(TreeNode *node, bool local);

void generateFuncDeclaration(TreeNode *node, bool local);

void generateReturn(TreeNode *node);

void generateAssign(TreeNode *node, bool local);

void generateIf(TreeNode *node, bool local);

void generateLabel(char *label);

void generateWhile(TreeNode *node, bool local);

int generateExpression(TreeNode *node, bool local);

void generateDeclaration(TreeNode *node, bool local);

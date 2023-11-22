#include <stdio.h>
#include <stdbool.h>

extern FILE *f;
extern int labelId;

void generateFuncCall(TreeNode *node, bool local);

void generateFuncDeclaration(TreeNode *node, bool local);

void generateReturn(TreeNode *node);

void generateAssign(TreeNode *node, bool local);

void generateIf(TreeNode *node, bool local);

void generateLabel(char *label);

void generateWhile(TreeNode *node, bool local);

void generateExpression(TreeNode *node, bool local);

void generateDeclaration(TreeNode *node, bool local);

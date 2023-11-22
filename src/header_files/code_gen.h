#include <stdio.h>
#include <stdbool.h>

extern FILE *f;
extern int labelId;

void generateFuncCall(TreeNode *node);

void generateFuncDeclaration(TreeNode *node);

void generateReturn(TreeNode *node);

void generateAssign(TreeNode *node, bool declaration, bool local);

void generateIf(TreeNode *node, bool local, bool ifElse);

void generateLabel(char *label);

void generateWhile(TreeNode *node, bool local);

#include "header_files/codeGen.h"
#include "header_files/parser.h"

FILE *file;
int labelId = 0;


void generateFuncCall(TreeNode *node) {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }
    
    fprintf(file, "CREATEFRAME\n"); 

    for (unsigned i = 0; i < node->numChildren; i++)
    {
        fprintf(file, "DEFVAR TF@%%%d\n", i);
        fprintf(file, "MOVE TF@%%%d %s\n", i, node->children[i]->children[0]->label);
    }

    fprintf(file, "CALL\n");
}

void generateFuncDef(TreeNode *node, bool local) {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }

    fprintf(file, "LABEL %s\n", node->children[1]->label);
    fprintf(file, "PUSHFRAME\n");
    if (node->children[3]->type != NODE_EPSILON)
        fprintf(file, "DEFVAR LF@%%retval\n");
    
    for (unsigned i = 0; i < node->children[2]->numChildren; i++)
    {
        fprintf(file, "DEFVAR LF@param%d\n", i);
        fprintf(file, "MOVE LF@param%d LF@%%%d\n", i, i);
    }
    
}

void generateReturn() {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }

    fprintf(file, "POPFRAME\n");
    fprintf(file, "RETURN\n");
}

void generateAssign(TreeNode *node, bool declaration, bool local) {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }

    char *frame = local ? "LF" : "GF";

    if (declaration)
    {
        fprintf(file, "DEFVAR %s@%s\n", frame, node->children[0]->label);
    }

    //fprintf(file, "MOVE %s@%s %s\n", frame, node->children[0]->label, node->children[1]->label);
}

void generateExpression(bool local) {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }

    char *frame = local ? "LF" : "GF";

}

void generateIf(TreeNode *node, bool local, bool ifElse) {
    file = fopen("out.ifjcode", "w");

    if (file == NULL)
    {
        return;
    }

    char *frame = local ? "LF" : "GF";

    fprintf(file, "DEFVAR %s@%%res%d\n", frame, labelId);

    // vyhodnocení výrazu

    fprintf(file, "JUMPIFNEQ %%if$%d@%%%sres%d bool@true\n", labelId, frame, labelId);
    labelId++;
}
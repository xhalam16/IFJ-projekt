#pragma once

#include <stdio.h>
#include "token.h"
#include "error.h"
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"

FILE *file;
error_code_t error;

typedef enum NodeType {
    NODE_PROGRAM,
    NODE_ASSIGN,
    NODE_DECLARATION,
    NODE_DECLARATION_FUNCTION,
    NODE_EXPRESSION,
    NODE_IF_STATEMENT,
    NODE_ELSE_STATEMENT,
    NODE_KEYWORD_LET,
    NODE_IDENTIFIER,
    NODE_KEYWORD_FUNC,
    NODE_FUNCTION_CALL,
    NODE_FUNCTION_PARAMS,
    NODE_FUNCTION_PARAM,
    NODE_FUNCTION_PARAM_TYPE,
    NODE_PARAM_VALUE,
    NODE_PARAM_LIST,
    NODE_INT,
    NODE_DOUBLE,
    NODE_STRING,
    NODE_NIL,
    NODE_EPSILON,
    NODE_UNDERSCORE
} NodeType;

typedef struct TreeNode {
    NodeType type;
    bool terminal;
    struct TreeNode **children;
    unsigned numChildren;
    char *label;
    local_symtable_t *local_symtable;
} TreeNode;

void dispose(TreeNode *parseTree);

TreeNode *createNewNode(TreeNode *parent, NodeType type, bool terminal);

bool parse(TreeNode *startNeterminal, bool innerBlock);


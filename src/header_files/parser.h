#pragma once

#include <stdio.h>
#include "token.h"
#include "error.h"
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

typedef enum NodeType {
    NODE_START,
    NODE_ASSIGN,
    NODE_EXPRESSION,
    NODE_FUNCTION_CALL,
    NODE_IDENTIFIER,
} NodeType;

typedef struct TreeNode {
    NodeType type;
    TreeNode *children;
    unsigned numChildren;
} TreeNode;

void dispose(TreeNode *node);

TreeNode *createNewNode(TreeNode *node);

token_t emptyLines(token_t token);

bool parseKeyWord(error_code_t *error);

bool parseParameter(TreeNode *node, error_code_t *error, bool assign);

bool parseAssign(error_code_t *error);

bool parseFuncCall(TreeNode *node, error_code_t *error);

bool parseId(TreeNode *node, error_code_t *error, bool declaration);

bool parse(TreeNode *node, error_code_t *error);


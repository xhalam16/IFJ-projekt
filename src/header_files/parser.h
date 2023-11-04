#pragma once

#include <stdio.h>
#include "token.h"
#include "error.h"
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

FILE *file;

typedef enum NodeType {
    NODE_PROGRAM,
    NODE_ASSIGN,
    NODE_EXPRESSION,
    NODE_IF,
    NODE_IDENTIFIER,
    NODE_FUNCTION_CALL,
    NODE_FUNCTION_PARAMS,
    NODE_FUNCTION_PARAM,
    NODE_INT,
    NODE_DOUBLE,
    NODE_STRING,
    NODE_NIL
} NodeType;

typedef struct TreeNode {
    NodeType type;
    bool terminal;
    struct TreeNode *children;
    unsigned numChildren;
} TreeNode;

void dispose(TreeNode *node);

TreeNode *createNewNode(TreeNode *node, error_code_t *error);

void skipEmptyLines(token_t *token);

bool parseDeclaration(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error);

bool parseCondition(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error, bool inParenthesis);

bool parseLeftBrace(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error);

bool parseComma(TreeNode *funcParams, error_code_t *error);

bool parseKeyWord(error_code_t *error);

bool parseParameter(TreeNode *node, error_code_t *error);

bool parseAssign(error_code_t *error);

bool parseFuncCall(TreeNode *node, error_code_t *error);

bool parseId(TreeNode *node, error_code_t *error, bool declaration);

bool parse(TreeNode *startNeterminal, error_code_t *error, bool innerBlock);


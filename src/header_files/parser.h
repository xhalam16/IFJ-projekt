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
    NODE_DECLARATION,
    NODE_EXPRESSION,
    NODE_IF_STATEMENT,
    NODE_ELSE_STATEMENT,
    NODE_KEYWORD_LET,
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
    struct TreeNode **children;
    unsigned numChildren;
} TreeNode;

void dispose(TreeNode *node, unsigned numChildren);

TreeNode *createNewNode(TreeNode *node, error_code_t *error);

int skipEmptyLines(token_t *token, error_code_t *error);

bool parseDeclaration(TreeNode *neterminal, error_code_t *error);

bool parseCondition(TreeNode *neterminal, error_code_t *error, bool inParenthesis);

bool parseKeyWord(error_code_t *error);

bool parseParameters(TreeNode *node, error_code_t *error);

bool parseAssign(TreeNode *assign, error_code_t *error);

bool parseFuncCall(TreeNode *node, error_code_t *error);

bool parseId(TreeNode *node, error_code_t *error, bool declaration);

bool parseIfStatement(TreeNode *node, error_code_t *error);

bool parse(TreeNode *startNeterminal, error_code_t *error, bool innerBlock);


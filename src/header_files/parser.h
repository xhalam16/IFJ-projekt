#pragma once

#include <stdio.h>
#include "token.h"
#include "error.h"
#include <string.h>
#include <stdbool.h>
#include "scanner.h"

typedef struct TreeNode {
    token_type_t type;
    struct TreeNode* children;
} TreeNode;

void dispose(TreeNode *node);

TreeNode *createNewNode(token_t *token);

bool parseKeyWord(error_code_t *error);

bool parseAssign(error_code_t *error);

bool parseId(error_code_t *error);

bool parse(token_t *token, error_code_t *error);


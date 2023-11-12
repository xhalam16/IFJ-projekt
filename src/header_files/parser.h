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
extern global_symtable *global_table;


typedef enum NodeType {
    NODE_PROGRAM,
    NODE_BODY,
    NODE_ASSIGN,
    NODE_DECLARATION,
    NODE_DECLARATION_FUNCTION,
    NODE_EXPRESSION,
    NODE_IF_STATEMENT,
    NODE_ELSE_STATEMENT,
    NODE_RETURN,
    NODE_KEYWORD_LET,
    NODE_KEYWORD_VAR,
    NODE_KEYWORD_RETURN,
    NODE_KEYWORD_FUNC,
    NODE_FUNCTION_CALL,
    NODE_FUNCTION_PARAM,
    NODE_PARAM_VALUE,
    NODE_PARAM_LIST,
    NODE_IDENTIFIER,
    NODE_INT,
    NODE_DOUBLE,
    NODE_STRING,
    NODE_NIL,
    NODE_INT_NILABLE,
    NODE_DOUBLE_NILABLE,
    NODE_STRING_NILABLE,
    NODE_DATATYPE_INT,
    NODE_DATATYPE_DOUBLE,
    NODE_DATATYPE_STRING,
    NODE_DATATYPE_INT_NILABLE,
    NODE_DATATYPE_DOUBLE_NILABLE,
    NODE_DATATYPE_STRING_NILABLE,
    NODE_EPSILON,
    NODE_UNDERSCORE
} NodeType;

typedef struct t_n_mapping {
    token_type_t t_value;
    NodeType n_value;
} Token_to_node;

typedef struct n_d_mapping {
    NodeType n_value;
    data_type_t d_value;
} Node_to_data;

extern const Node_to_data node_to_data[];
extern const Token_to_node token_to_node[];

typedef struct TreeNode {
    NodeType type;
    bool terminal;
    struct TreeNode **children;
    unsigned numChildren;
    char *label;
    local_symtable *local_symtable;
} TreeNode;

void dispose(TreeNode *parseTree);

TreeNode *createNewNode(TreeNode *parent, NodeType type, bool terminal);

data_type_t node_type_to_data(NodeType n_type);

bool parse(TreeNode *startNeterminal, bool inBlock, bool inFunction, bool voidFunction);

// for debug
void print_global_table(global_symtable *table);

void printTree(TreeNode *node);


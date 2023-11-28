/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: parser.h
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 */


#pragma once

#include <stdio.h>
#include "token.h"
#include "error.h"
#include <string.h>
#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"
#include "stack.h"
#include "dynamic_array.h"


static error_code_t error;
static unsigned inBlock;
static bool inFunction;
static local_symtable *local_table;
static unsigned inBlockCounter;

extern FILE *file;
extern global_symtable *global_table;
extern Stack *stack_of_local_tables;

#define NODE_CHILDREN_ARRAY_CAPACITY 8 // minimální počet, po kterých se bude realokovat pole dětí uzlu

typedef enum NodeType
{
    NODE_PROGRAM,                 // 0
    NODE_BODY,                    // 1
    NODE_BODY_END,                // 2
    NODE_ASSIGN,                  // 3
    NODE_DECLARATION,             // 4
    NODE_DECLARATION_FUNCTION,    // 5
    NODE_EXPRESSION,              // 6
    NODE_IF_STATEMENT,            // 7
    NODE_WHILE,                   // 8
    NODE_RETURN,                  // 9
    NODE_KEYWORD_LET,             // 10
    NODE_GUARD_LET,               // 11
    NODE_KEYWORD_VAR,             // 12
    NODE_KEYWORD_RETURN,          // 13
    NODE_KEYWORD_FUNC,            // 14
    NODE_FUNCTION_CALL,           // 15
    NODE_FUNCTION_PARAM,          // 16
    NODE_PARAM_VALUE,             // 17
    NODE_PARAM_LIST,              // 18
    NODE_IDENTIFIER,              // 19
    NODE_INT,                     // 20
    NODE_DOUBLE,                  // 21
    NODE_STRING,                  // 22
    NODE_NIL,                     // 23
    NODE_INT_NILABLE,             // 24
    NODE_DOUBLE_NILABLE,          // 25
    NODE_STRING_NILABLE,          // 26
    NODE_OPERATOR_ADD,            // 27
    NODE_OPERATOR_SUB,            // 28
    NODE_OPERATOR_MUL,            // 29
    NODE_OPERATOR_DIV,            // 30
    NODE_OPERATOR_BELOW,          // 31
    NODE_OPERATOR_ABOVE,          // 32
    NODE_OPERATOR_BEQ,            // 33
    NODE_OPERATOR_AEQ,            // 34
    NODE_OPERATOR_EQUAL,          // 35
    NODE_OPERATOR_NEQ,            // 36
    NODE_OPERATOR_NIL_COALESCING, // 37
    NODE_OPERATOR_UNARY,          // 38
    NODE_LEFT_PARENTHESIS,        // 39
    NODE_RIGHT_PARENTHESIS,       // 40
    NODE_RIGHT_BRACE,             // 41
    NODE_LEFT_BRACE,              // 42
    NODE_EOL,                     // 43
    NODE_DATATYPE_INT,            // 44
    NODE_DATATYPE_DOUBLE,         // 45
    NODE_DATATYPE_STRING,         // 46
    NODE_DATATYPE_INT_NILABLE,    // 47
    NODE_DATATYPE_DOUBLE_NILABLE, // 48
    NODE_DATATYPE_STRING_NILABLE, // 49
    NODE_EPSILON,                 // 50
    NODE_UNDERSCORE,              // 51
    NODE_SHIFTER                  // 52
} NodeType;

typedef enum RuleType
{
    RULE_ID,
    RULE_PARENTHESES,
    RULE_ADD,
    RULE_SUB,
    RULE_MUL,
    RULE_DIV,
    RULE_COALESCING,
    RULE_UNARY,
    RULE_BELOW,
    RULE_ABOVE,
    RULE_BEQ,
    RULE_AEQ,
    RULE_EQUAL,
    RULE_NEQ,
} RuleType;

typedef struct t_n_mapping
{
    token_type_t t_value;
    NodeType n_value;
} Token_to_node;

typedef struct n_d_mapping
{
    NodeType n_value;
    data_type_t d_value;
} Node_to_data;

typedef struct tt_i_mapping
{
    NodeType n_value;
    unsigned i_value;
} NodeTypeToIndex;

extern const Node_to_data node_to_data[];
extern const Token_to_node token_to_node[];

typedef struct TreeNode
{
    NodeType type;
    bool terminal;
    struct TreeNode **children;
    unsigned numChildren;
    char *label;
    local_symtable *local_symtable;
    token_value_t token_value;
} TreeNode;

void dispose(TreeNode *parseTree);

TreeNode *createNewNode(TreeNode *parent, NodeType type, bool terminal);

data_type_t node_type_to_data(NodeType n_type);

bool parse(TreeNode *startNeterminal);

// for debug
void print_global_table(global_symtable *table);

void print_local_table(local_symtable *table);

// void printTree(TreeNode *node);

#pragma once

enum token_type
{
    TOKEN_IDENTIFIER, // id
    TOKEN_KEYWORD_IF, // keyword
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_VAR,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_FUNC,

    TOKEN_DATATYPE,
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_STRING,
    TOKEN_NIL,

    TOKEN_OPERATOR_ADD,    // +
    TOKEN_OPERATOR_SUB,    // -
    TOKEN_OPERATOR_MUL,    // *
    TOKEN_OPERATOR_DIV,    // /
    TOKEN_OPERATOR_BELOW,  // <
    TOKEN_OPERATOR_ABOVE,  // >
    TOKEN_OPERATOR_BEQ,    // <=
    TOKEN_OPERATOR_AEQ,    // >=
    TOKEN_OPERATOR_EQUAL,  // ==
    TOKEN_OPERATOR_NEQ,    // !=
    TOKEN_OPERATOR_AND,    // &&
    TOKEN_OPERATOR_OR,     // ||
    TOKEN_OPERATOR_ASSIGN, // =
    TOKEN_OPERATOR_UNARY,  // !
    TOKEN_EOL,
    TOKEN_EOF,

    TOKEN_LEFT_PARENTHESIS,     // (
    TOKEN_RIGHT_PARENTHESIS,    // )
    TOKEN_LEFT_BRACE,           // {
    TOKEN_RIGHT_BRACE,          // }
    TOKEN_LEFT_SQUARE_BRACKET,  // [
    TOKEN_RIGHT_SQUARE_BRACKET, // ]
    TOKEN_COLON,                // :
    TOKEN_COMMA,                // ,
    TOKEN_ARROW,                // ->
    TOKEN_UNDERSCORE,           // _

    TOKEN_SEMICOLON, // ;

    TOKEN_UNKNOWN, // neznámý token
    TOKEN_ERROR    // chyba
};

typedef enum token_type token_type_t;

typedef struct token_value
{
    int int_value;
    double double_value;
    char *string_value;
} token_value_t;

typedef struct token
{
    token_type_t type;
    token_value_t value;
} token_t;
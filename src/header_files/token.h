#pragma once

enum token_type {
    TOKEN_IDENTIFIER, // id
    TOKEN_KEYWORD, // keyword


    TOKEN_DATATYPE,
    TOKEN_IMMEDIATE_OPERAND, 

    TOKEN_OPERATOR_ASSIGN, // =
    TOKEN_OPERATOR_BINARY, // + - * / % < > <= >= == != && ||
    TOKEN_OPERATOR_UNARY, // ! -

    TOKEN_EOL,
    TOKEN_EOF,


    TOKEN_LEFT_PARENTHESIS, // (
    TOKEN_RIGHT_PARENTHESIS, // )
    TOKEN_LEFT_BRACE, // {
    TOKEN_RIGHT_BRACE, // }
    TOKEN_LEFT_SQUARE_BRACKET, // [
    TOKEN_RIGHT_SQUARE_BRACKET, // ]
    TOKEN_COLON, // :
    TOKEN_COMMA, // ,
    TOKEN_ARROW, // ->


    TOKEN_SEMICOLON, // ;

    TOKEN_UNKNOWN, // neznámý token
    TOKEN_ERROR // chyba
};


typedef enum token_type token_type_t;


typedef struct token_value {
    int int_value;
    double double_value;
    char *string_value;
} token_value_t;

typedef struct token {
    token_type_t type;
    token_value_t value;
} token_t;
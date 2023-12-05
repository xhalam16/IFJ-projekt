/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: token.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */

#pragma once
#include "dynamic_buffer.h"


/**
 * @brief Enum pro typ tokenu
*/
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

    TOKEN_DATATYPE_INT,
    TOKEN_DATATYPE_DOUBLE,
    TOKEN_DATATYPE_STRING,
    TOKEN_DATATYPE_INT_NILABLE,
    TOKEN_DATATYPE_DOUBLE_NILABLE,
    TOKEN_DATATYPE_STRING_NILABLE,
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
    TOKEN_OPERATOR_NIL_COALESCING, // ??
    TOKEN_OPERATOR_ASSIGN, // =
    TOKEN_OPERATOR_UNARY,  // !
    TOKEN_EOL,
    TOKEN_EOF,

    TOKEN_LEFT_PARENTHESIS,     // (
    TOKEN_RIGHT_PARENTHESIS,    // )
    TOKEN_LEFT_BRACE,           // {
    TOKEN_RIGHT_BRACE,          // }
    TOKEN_COLON,                // :
    TOKEN_COMMA,                // ,
    TOKEN_ARROW,                // ->
    TOKEN_UNDERSCORE,           // _

    TOKEN_UNKNOWN, // neznámý token (lex error)
    TOKEN_ERROR, // chyba (např. malloc)
    TOKEN_NONE // syntax analysis - skip token
};


typedef enum token_type token_type_t;


/**
 * @brief Union pro uložení hodnoty tokenu
 * @param int_value hodnota tokenu typu int
 * @param double_value hodnota tokenu typu double
 * @param string_value hodnota tokenu typu string
 * @typedef token_value_t
*/
typedef union token_value
{
    int int_value;
    double double_value;
    DynamicBuffer* string_value;
} token_value_t;


/**
 * @brief Struktura tokenu
 * @param type typ tokenu
 * @param value hodnota tokenu
 * @param source_value hodnota tokenu ve zdrojovém kódu
 * @typedef token_t
*/
typedef struct token
{
    token_type_t type;
    token_value_t value;
    DynamicBuffer* source_value;
} token_t;


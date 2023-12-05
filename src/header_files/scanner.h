/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: scanner.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "error.h"
#include <math.h>


/**
 * @brief Private global variable for the scanner
 * @param in_block_comment_global Whether the scanner is currently in a block comment
*/
static bool in_block_comment_global;;


/**
 * @brief Struct for mapping keywords to token types
 * @param keyword The keyword
 * @param type The corresponding token type
 * @typedef keyword_token_type_t
*/
typedef struct keyword_type
{
    const char *keyword;
    token_type_t type;
} keyword_token_type_t;


/**
 * @brief Extern variable for mapping keywords to token types
*/
extern const keyword_token_type_t keywords_map[];




/**
 * @brief Macro for getting the size of the keywords_map array
*/
#define KEYWORD_COUNT sizeof(keywords_map) / sizeof(keywords_map[0])



/**
 * @brief Function returning the token type corresponding to the given keyword
 * @param keyword The keyword to get the token type for
 * @return The token type corresponding to the given keyword
*/
token_type_t keyword_2_token_type(char *keyword);



/**
 * @brief Function for getting the next character from the source file
 * @param source_file The source file
 * @return The next character from the source file
*/
int get_char(FILE *source_file);


/**
 * @brief Main function of the scanner, returns the next token from the source file
 * @param source_file The source file
 * @return The next token from the source file
*/
token_t get_token(FILE *source_file);


/**
 * @brief Returns the next token from the source file without consuming it
 * @param source_file The source file
 * @return The next token from the source file
*/
token_t peek_token(FILE *source_file);

/**
 * @brief Ungets the given token back to the source file
 * @param token The token to unget
 * @param source_file The source file
*/
void unget_token(token_t token, FILE *source_file);


/**
 * @brief Frees the given token
 * @param token The token to free
*/
void free_token(token_t token);









#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "error.h"
#include <math.h>

extern bool in_string_global;
extern bool in_block_comment_global;
extern bool in_multi_line_string_global;

typedef struct keyword_type
{
    const char *keyword;
    token_type_t type;
} keyword_token_type_t;


extern const keyword_token_type_t keywords_map[];

#define KEYWORD_COUNT sizeof(keywords_map) / sizeof(keywords_map[0])

token_type_t keyword_2_token_type(char *keyword);
bool is_escape_sequence(char c);
int skip_block_comment(FILE *source_file);
int skip_line_comment(FILE *source_file);
bool word_is_keyword(char *word);
bool word_is_identifier(char *word);
bool word_is_keyword_datatype(char *word);
int get_char(FILE *source_file);

void skip_whitespace(FILE *source_file);

token_t get_token(FILE *source_file);
void unget_token(token_t token, FILE *source_file);
token_t peek_token(FILE *source_file);








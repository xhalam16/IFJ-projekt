#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "error.h"
#include <math.h>

static bool in_string_global;
static bool in_block_comment_global;
static bool in_multi_line_string_global;

typedef struct keyword_type
{
    const char *keyword;
    token_type_t type;
} keyword_token_type_t;


extern const keyword_token_type_t keywords_map[];

#define KEYWORD_COUNT sizeof(keywords_map) / sizeof(keywords_map[0])

token_type_t keyword_2_token_type(char *keyword);
int get_char(FILE *source_file);


token_t get_token(FILE *source_file);
void unget_token(token_t token, FILE *source_file);


void free_token(token_t token);

token_t peek_token(FILE *source_file);








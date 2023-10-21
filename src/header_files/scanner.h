#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "error.h"


typedef struct keyword_type
{
    const char *keyword;
    token_type_t type;
} keyword_token_type_t;


const keyword_token_type_t keywords_map[] = {
    {"func", TOKEN_KEYWORD_FUNC},
    {"Double", TOKEN_DATATYPE_DOUBLE},
    {"String", TOKEN_DATATYPE_STRING},
    {"Int", TOKEN_DATATYPE_INT},
    {"if", TOKEN_KEYWORD_IF},
    {"else", TOKEN_KEYWORD_ELSE},
    {"return", TOKEN_KEYWORD_RETURN},
    {"nil", TOKEN_NIL},
    {"let", TOKEN_KEYWORD_LET},
    {"var", TOKEN_KEYWORD_VAR},
    {"while", TOKEN_KEYWORD_WHILE},
    {"Int?", TOKEN_DATATYPE_INT_NILABLE},
    {"Double?", TOKEN_DATATYPE_DOUBLE_NILABLE},
    {"String?", TOKEN_DATATYPE_STRING_NILABLE}

};

#define KEYWORD_COUNT sizeof(keywords_map) / sizeof(keywords_map[0])


bool word_is_keyword(char *word);

bool token_is_keyword(token_t token);

int get_char(FILE *source_file);

void skip_whitespace(FILE *source_file);

token_t get_token(FILE *source_file);






#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"



const char * keyword_strings[] = {
    // keywords for Swift language
    "func",
    "Double",
    "String",
    "Int",
    "if",
    "else",
    "for",
    "return",
    "nil",
    "let",
    "var",
    "while",
    "Int?",
    "Double?",
    "String?"
    
};

#define KEYWORD_COUNT sizeof(keyword_strings) / sizeof(keyword_strings[0])


bool word_is_keyword(char *word);

bool token_is_keyword(token_t token);

int get_char(FILE *source_file);

void skip_whitespace(FILE *source_file);

token_t get_token(FILE *source_file);






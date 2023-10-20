#include "header_files/scanner.h"


bool word_is_keyword(char *word){
    for (int i = 0; i < KEYWORD_COUNT; i++)
    {
        if(strcmp(word, keyword_strings[i]) == 0){
            return true;
        }
        
        
    }

    return false;
}

bool word_is_identifier(char *word){
    if(isalpha(word[0]) || word[0] == '_'){
        if(strlen(word) == 1 && word[0] == '_'){
            return false;
        }
        return true;
    }

    return false;
}

bool word_is_keyword_datatype(char *word){
    const char *keyword_datatypes[] = {
        "Int",
        "Double",
        "String",
        "Int?",
        "Double?",
        "String?"
    };
    
    for (int i = 0; i < sizeof(keyword_datatypes) / sizeof(keyword_datatypes[0]); i++)
    {
        if(strcmp(word, keyword_datatypes[i]) == 0){
            return true;
        }
    }

    return false;
}



void skip_whitespace(FILE *source_file){
    int c;
    while((c = get_char(source_file)) != EOF){
        if (isspace(c)){
            continue;
        }
        else{
            ungetc(c, source_file);
            break;
        }
    }
}


int get_char(FILE *source_file){
    return fgetc(source_file);
}

token_t get_token(FILE *source_file){
    token_t token;
    token.type = TOKEN_UNKNOWN;
    token.value.int_value = 0;
    token.value.double_value = 0;
    token.value.string_value = NULL;

    skip_whitespace(source_file);
   int c = get_char(source_file);

   // TODO throw error?
    if (c == EOF){
        token.type = TOKEN_EOF;
        return token;
    }

    if(isdigit(c)){
    // určitě nemůže být klíčové slovo a identifikátor
        token.type = TOKEN_IMMEDIATE_OPERAND;
        token.value.int_value = c - '0';
        while(isdigit(c = get_char(source_file))){
            token.value.int_value = token.value.int_value * 10 + (c - '0');
        }

        // todo vyresit desetinna cisla ve tvaru cisloe+cislo
        if (c == '.'){
            token.type = TOKEN_IMMEDIATE_OPERAND;
            token.value.double_value = token.value.int_value;
            double decimal = 0.1;
            while(isdigit(c = get_char(source_file))){
                token.value.double_value += (c - '0') * decimal;
                decimal /= 10;
            }

        }
        else{
            ungetc(c, source_file);
        }
        return token;

    }else if(isalpha(c) || c == '_'){
        // could be keyword or identifier
        char buffer[256];
        int i = 0;
        
        // todo dynamický buffer, ošetřit přetečení
        while (isalnum(c) || c == '_' || c == '?')
        {
            buffer[i++] = c;
            c = get_char(source_file);
        }

        buffer[i] = '\0';
        printf("buffer: %s\n", buffer);

       
        if(word_is_keyword(buffer)){
            token.type = word_is_keyword_datatype(buffer) ? TOKEN_DATATYPE : TOKEN_KEYWORD;
            token.value.string_value = malloc(strlen(buffer) + 1);
            strcpy(token.value.string_value, buffer);
        }
         else if(word_is_identifier(buffer)){
            token.type = TOKEN_IDENTIFIER;
            token.value.string_value = malloc(strlen(buffer) + 1);
            strcpy(token.value.string_value, buffer);
        }
        else{
            token.type = TOKEN_UNKNOWN;
        }


    return token;

    } else if(c == '='){
        token.type = TOKEN_OPERATOR_ASSIGN;
        return token;

    }
    
    
    else{

    ungetc(c, source_file);
    }




   token.type = TOKEN_EOF;
    return token;
    

}



// DEBUG PURPOSE
int main(){
    FILE *source_file = fopen("test.txt", "r");
    token_t token;
    while((token = get_token(source_file)).type != TOKEN_EOF){
        printf("type: %d\n", token.type);
        printf("int_value: %d\n", token.value.int_value);
        printf("double_value: %f\n", token.value.double_value);
        printf("string_value: %s\n", token.value.string_value);
        printf("\n");
    }
    fclose(source_file);
    return 0;
}

// end DEBUG PURPOSE
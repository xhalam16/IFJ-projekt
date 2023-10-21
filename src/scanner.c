#include "header_files/scanner.h"

token_type_t keyword_2_token_type(char *keyword){
    for (int i = 0; i < KEYWORD_COUNT; i++)
    {
        if(strcmp(keyword, keywords_map[i].keyword) == 0){
            return keywords_map[i].type;
        }
        
        
    }

    return TOKEN_UNKNOWN;
}


int skip_block_comment(FILE *source_file){
    int c;
    while((c = get_char(source_file)) != EOF){
        if(c == '*'){
            c = get_char(source_file);
            if(c == '/'){
                return 0;
            }else{
                ungetc(c, source_file);
            }
        }
    }

    if(c == EOF)
        return EOF;

    return 1;
}

int skip_line_comment(FILE *source_file){
    int c;
    while((c = get_char(source_file)) != EOF){
        if(c == '\n'){
            return 0;
        }
    }

    if(c == EOF)
        return EOF;

    return 1;
}

bool word_is_keyword(char *word){
    for (int i = 0; i < KEYWORD_COUNT; i++)
    {
        if(strcmp(word, keywords_map[i].keyword) == 0){
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
        if (isspace(c) && c != '\n'){
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
   if(c == '\n'){
         token.type = TOKEN_EOL;
         return token;
   }
   // TODO throw error?
    if (c == EOF){
        token.type = TOKEN_EOF;
        return token;
    }
    

    if(isdigit(c)){
    // určitě nemůže být klíčové slovo a identifikátor
        token.type = TOKEN_INT;
        token.value.int_value = c - '0';
        while(isdigit(c = get_char(source_file))){
            token.value.int_value = token.value.int_value * 10 + (c - '0');
        }

        // todo vyresit desetinna cisla ve tvaru cisloe+cislo
        if (c == '.'){
            token.type = TOKEN_DOUBLE;
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
            token_type_t type = keyword_2_token_type(buffer);
            token.type = type;
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
        // could be assignment or comparison
        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_EQUAL;
            token.value.string_value = malloc(3);
            strcpy(token.value.string_value, "==");
        }
        else{
            token.type = TOKEN_OPERATOR_ASSIGN;
            token.value.string_value = malloc(2);
            strcpy(token.value.string_value, "=");
        }
        
        return token;

    } else if(c == '!'){
        // could be either unary operator or comparison (not equal)

        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_NEQ;
            token.value.string_value = malloc(3);
            strcpy(token.value.string_value, "!=");
        }
        else{
            token.type = TOKEN_OPERATOR_UNARY;
            token.value.string_value = malloc(2);
            strcpy(token.value.string_value, "!");
        }
    }else if(c == '?'){
        // has to be another ? else error (since identifiers are already checked) 
        // ?? - null coalescing operator
        c = get_char(source_file);
        if(c == '?'){
            token.type = TOKEN_OPERATOR_NIL_COALESCING;
            token.value.string_value = malloc(3);
            strcpy(token.value.string_value, "??");
        }
        else{
            token.type = TOKEN_UNKNOWN;
        }
    } else if(c == '/'){
        // could be either line comment, division or block comment
        c = get_char(source_file);

        if(c == '/'){
            // comment
            // skip until end of line
           int res = skip_line_comment(source_file);
              if(res == EOF){
                token.type = TOKEN_EOF;
            }else if(res == 1){
                token.type = TOKEN_ERROR;
            }else{
                token.type = TOKEN_NONE;
            }
               
        }
        else if(c == '*'){
            /* block comment
             skip until end of block comment */
            int result = skip_block_comment(source_file);
            if(result == EOF){
                token.type = TOKEN_EOF;
            }else if(result == 1){
                token.type = TOKEN_ERROR;
            }else{
                token.type = TOKEN_NONE;
            }


        }
        else{
            // division
            token.type = TOKEN_OPERATOR_DIV;
            token.value.string_value = malloc(2);
            strcpy(token.value.string_value, "/");
           
        }

    }
    
    
    else{

    //ungetc(c, source_file);
    }




    return token;
    


}




int main(){
    FILE *source_file = fopen("test.txt", "r");
    token_t token;
    while((token = get_token(source_file)).type != TOKEN_EOF){
        
        printf("type: %s\n", token_type_string_values[token.type]);
        printf("int_value: %d\n", token.value.int_value);
        printf("double_value: %f\n", token.value.double_value);
        printf("string_value: %s\n", token.value.string_value);
        printf("\n");
    }
    fclose(source_file);
    return 0;
}

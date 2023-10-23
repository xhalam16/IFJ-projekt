#include "header_files/scanner.h"
#include "header_files/dynamic_buffer.h"

bool in_string_global = false;

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
    bool next_number_negative = false;
   
    DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));
    if(buffer == NULL){
        token.type = TOKEN_ERROR;
        return token;
    }

    if(init_buffer(buffer, BUFFER_INIT_CAPACITY) != ERR_CODE_OK){
        token.type = TOKEN_ERROR;
        return token;
    }
    token.type = TOKEN_UNKNOWN;
    token.value.int_value = 0;
    token.value.double_value = 0;
    token.value.string_value = buffer;

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

    if(c == '-'){
        // this branch either sets that next number will be negative or arithmetic minus
        // it could also be arrow operator



        c = get_char(source_file);
        if(isdigit(c)){
             next_number_negative = true;
           // ungetc(c, source_file);
        }else if(c != '>'){
            // arithmetic minus
            token.type = TOKEN_OPERATOR_SUB;
            buffer_append_string(buffer, "-");
            ungetc(c, source_file);
            return token;
        }else{
            token.type = TOKEN_ARROW;
            buffer_append_string(buffer, "->");
            return token;
        }
    }

    if(c == '"'){
       
        // this branch either sets that string is being loaded (string literal)
        token.type = TOKEN_DOUBLE_QUOTE;
        buffer_append_string(buffer, "\"");
        c = get_char(source_file);
        if(c != '"'){
            // we return the token quote and start loading string
            in_string_global = true;
            ungetc(c, source_file);

        }else if(c == '"' && in_string_global){
            // end of string
         
            in_string_global = false;
            token.type = TOKEN_DOUBLE_QUOTE;
            buffer_append_string(buffer, "\"");
            
        }else{
            ungetc(c, source_file);
        }
        
            return token;
    }



    
    if(isdigit(c)){
        // určitě nemůže být klíčové slovo a identifikátor
        // this branch scans for numbers
    
        do
        {
           
            buffer_append_char(buffer, c);
         
            c = get_char(source_file);
        } while (isdigit(c));

        

        if(c == '.'){
            // double
            // todo specialni pripad double ve formatu cislo e sign cislo
            buffer_append_char(buffer, c);
            c = get_char(source_file);
            while (isdigit(c))
            {
                buffer_append_char(buffer, c);
                c = get_char(source_file);
            }
            ungetc(c, source_file);
            token.type = TOKEN_DOUBLE;
            double value = strtod(buffer->buffer, NULL);
            if(next_number_negative){
                value *= -1;
                buffer_insert_char_beggining(buffer, '-');
            }

            token.value.double_value = value;
            
        }
        else{
            // int
          
            ungetc(c, source_file);
            token.type = TOKEN_INT;

            int value = (int)strtol(buffer->buffer, NULL, 10);
            if(next_number_negative){
                value *= -1;
                buffer_insert_char_beggining(buffer, '-');
             
            }
            token.value.int_value = value;

        }
        


        

    }else if(isalpha(c) || c == '_'){
        // could be keyword or identifier
        // need to also check for ( and ) - function call/declaration
        buffer_clear(buffer);
        while (isalnum(c) || c == '_' || c == '?')
        {
            // todo ošetřit realloc fail => err_code_internal
            buffer_append_char(buffer, c);
            c = get_char(source_file);
        }

        if(word_is_keyword(buffer->buffer)){
            token_type_t type = keyword_2_token_type(buffer->buffer);
            token.type = type;
           // token.value.string_value = buffer;
        }
         else if(word_is_identifier(buffer->buffer)){
            token.type = TOKEN_IDENTIFIER;
            //token.value.string_value = buffer;
        }
        else if(buffer_equals_string(buffer, "_")){
            // this covers the case of _ being used as a placeholder in function declaration
            token.type = TOKEN_UNDERSCORE;
          
        }
        ungetc(c, source_file);


    } else if(c == '='){
        // could be assignment or comparison
        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_EQUAL;
           // token.value.string_value = buffer;
            buffer_append_string(buffer, "==");
        }
        else{
            token.type = TOKEN_OPERATOR_ASSIGN;
           // token.value.string_value = buffer;
            buffer_append_string(buffer, "=");
        }
        
        

    } else if(c == '!'){
        // could be either unary operator or comparison (not equal)

        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_NEQ;
            buffer_append_string(buffer, "!=");
        }
        else{
            token.type = TOKEN_OPERATOR_UNARY;
            buffer_append_string(buffer, "!");
            ungetc(c, source_file);
        }
    }else if(c == '?'){
        // has to be another ? else error (since identifiers are already checked) 
        // ?? - null coalescing operator
        c = get_char(source_file);
        if(c == '?'){
            token.type = TOKEN_OPERATOR_NIL_COALESCING;
            buffer_append_string(buffer, "??");
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
            buffer_append_string(buffer, "/");
           
        }

    } else if(c == '+'){
        // has to be addition
        token.type = TOKEN_OPERATOR_ADD;
        buffer_append_string(buffer, "+");

    }else if(c == '*'){
        // has to be multiplication
        token.type = TOKEN_OPERATOR_MUL;
        buffer_append_string(buffer, "*");
    }else if(c == '('){
        // has to be left parenthesis
        token.type = TOKEN_LEFT_PARENTHESIS;
        buffer_append_string(buffer, "(");
    }else if(c == ')'){
        // has to be right parenthesis
        token.type = TOKEN_RIGHT_PARENTHESIS;
        buffer_append_string(buffer, ")");
    }else if(c == '{'){
        // has to be left curly bracket
        token.type = TOKEN_LEFT_BRACE;
        buffer_append_string(buffer, "{");
    }else if(c == '}'){
        // has to be right curly bracket
        token.type = TOKEN_RIGHT_BRACE;
        buffer_append_string(buffer, "}");
    }else if(c == ':'){
        // has to be colon
        token.type = TOKEN_COLON;
        buffer_append_string(buffer, ":");
    }else if(c == ','){
        // has to be comma
        token.type = TOKEN_COMMA;
        buffer_append_string(buffer, ",");
    }
    else if(c == '<'){
        // could be either below or below or equal
        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_BEQ;
            buffer_append_string(buffer, "<=");
        }
        else{
            token.type = TOKEN_OPERATOR_BELOW;
            buffer_append_string(buffer, "<");
            ungetc(c, source_file);
        }
    } else if(c == '>'){
        // could be either above or above or equal
        c = get_char(source_file);
        if(c == '='){
            token.type = TOKEN_OPERATOR_AEQ;
            buffer_append_string(buffer, ">=");
        }
        else{
            token.type = TOKEN_OPERATOR_ABOVE;
            buffer_append_string(buffer, ">");
            ungetc(c, source_file);
        }
    }
    
    
    else{

        // todo throw lexical error (TOKEN_UNKNOWN is default)
       
    }

    


    return token;

    


}

void unget_token(token_t token, FILE *source_file){

    token_type_t type = token.type;
    
    if(type == TOKEN_EOF){
        ungetc(EOF, source_file);
        return;
    }
    if(token.type == TOKEN_EOL){
        ungetc('\n', source_file);
        return;
    }


    DynamicBuffer *buffer = token.value.string_value;
    if(buffer == NULL){
        return;
    }

    char *string_value = buffer->buffer;
    if(string_value == NULL){
        return;
    }

    int length = strlen(string_value);
    for (int i = length - 1; i >= 0; i--)
    {
        ungetc(string_value[i], source_file);
    }





    



}

token_t peek_token(FILE *source_file){
    token_t token = get_token(source_file);
    unget_token(token, source_file);
    return token;
}




int main(){
    FILE *source_file = fopen("test.txt", "r");
    token_t token;
    while((token = get_token(source_file)).type != TOKEN_EOF){
        
        printf("type: %s\n", token_type_string_values[token.type]);
        // printf("int_value: %d\n", token.value.int_value);
        // printf("double_value: %f\n", token.value.double_value);
        // printf("string_value: %s\n", token.value.string_value->buffer);
        printf("\n");
    }
    free_buffer(token.value.string_value);
    fclose(source_file);
    return 0;
}

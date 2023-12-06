/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: scanner.c
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#include "header_files/scanner.h"
#include "header_files/dynamic_buffer.h"

static bool in_block_comment_global = false;

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


token_type_t keyword_2_token_type(char *keyword)
{
    for (size_t i = 0; i < KEYWORD_COUNT; i++)
    {
        if (strcmp(keyword, keywords_map[i].keyword) == 0)
        {
            return keywords_map[i].type;
        }
    }

    return TOKEN_UNKNOWN;
}


/**
 * @brief Checks if character is valid escape sequence
*/
bool is_escape_sequence(char c)
{
    switch(c){
        case 'n':
        case 't':
        case '"':
        case '\\':
        case '0':
        case '\'':
        case 'r':
            return true;
        default:
            return false;
    }
}


/**
 * @brief Skips block comment
 * @returns 0 if block comment was successfully skipped, 1 if LEX error occured, EOF if EOF was reached
*/
int skip_block_comment(FILE *source_file)
{
    // when we enter, we already have / loaded
    int c;
    while ((c = get_char(source_file)) != EOF)
    {
        if (c == '*')
        {
            // potential end of block comment
            c = get_char(source_file);
            if (c == '/')
            {
                return 0;
            }
            else
            {
                ungetc(c, source_file);
            }
        }

        if (c == '/')
        {
            // another block comment may be inside this one
            c = get_char(source_file);
            if (c == '*')
            {
                // another block comment
                int result = skip_block_comment(source_file);
                if (result == EOF)
                {
                    return EOF;
                }
                else if (result == 1)
                {
                    return 1;
                }
            }
            else
            {
                ungetc(c, source_file);
            }
        }
    }

    if (c == EOF)
        return EOF;

    return 1;
}


/**
 * @brief Skips line comment
 * @returns true if line comment was successfully skipped, false as a default value
 * @note This function is called only when we already have // loaded
 * @note EOF is valid, since the comment can be at the end of file
*/
bool skip_line_comment(FILE *source_file)
{
    int c;
    while ((c = get_char(source_file)))
    {
        if (c == '\n' || c == EOF)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks if word is keyword
 * @returns true if word is keyword, false otherwise
 * @note This function uses the keywords_map array defined in this file
*/
bool word_is_keyword(char *word)
{
    for (size_t i = 0; i < KEYWORD_COUNT; i++)
    {
        if (strcmp(word, keywords_map[i].keyword) == 0)
        {
            return true;
        }
    }

    return false;
}


/**
 * @brief Checks if word is identifier
 * @returns true if word is identifier, false otherwise
*/
bool word_is_identifier(char *word)
{
    // we also need to check for ? since it is NOT valid identifier 
    // and it was loaded into buffer because of keyword Int?, Double? and String?
    for(int i = 0; i < strlen(word); i++){
        if(word[i] == '?'){
            return false;
        }
    }

    // if first char is not alpha or _, it is not valid identifier
    if (isalpha(word[0]) || word[0] == '_')
    {
        // only underscore is not valid identifier
        if (strlen(word) == 1 && word[0] == '_')
        {
            return false;
        }
        return true;
    }

    return false;
}


/**
 * @brief Checks if word is keyword datatype
 * @returns true if word is keyword datatype, false otherwise
*/
bool word_is_keyword_datatype(char *word)
{
    const char *keyword_datatypes[] = {
        "Int",
        "Double",
        "String",
        "Int?",
        "Double?",
        "String?"};

    for (int i = 0; i < (int)(sizeof(keyword_datatypes) / sizeof(keyword_datatypes[0])); i++)
    {
        if (strcmp(word, keyword_datatypes[i]) == 0)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief Skips whitespace characters in source file
*/
void skip_whitespace(FILE *source_file)
{
    int c;
    while ((c = get_char(source_file)) != EOF)
    {
        if (isspace(c) && c != '\n')
        {
            continue;
        }
        else
        {
            ungetc(c, source_file);
            break;
        }
    }
}


/**
 * @brief Checks if character is unicode escape sequence
*/
bool is_unicode_escape(char c)
{
    return c == 'u';
}

/**
 * @brief Creates escape sequence from character passed as parameter
 * @returns -1 if character is not valid escape sequence, otherwise returns the escape sequence
*/
int create_escape_sequence(char c)
{
    switch (c)
    {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case '"':
        return '"';
    case '\\':
        return '\\';
    case '0':
        return '\0';

    case '\'':
        return '\'';
    case 'r':
        return '\r';

    default:
        return -1;
    }

}


/**
 * @brief Converts string to double
 * @returns double value of string
 * @note This function is used to convert string to double when loading double from source file
 * @note This function accepts all formats of double, including scientific notation (e.g. 1.2e-3)
 * @note This function skips unnecessary zeroes
 * @note If an error occurs, it modifies error_code parameter and returns 0
*/
double string_to_double(char *string, bool positive_exponent, error_code_t *error_code)
{
    bool skip_zeroes = true;
    bool exponent_started = false;


    // we need to first take care of exponent
    // and then mantissa
    DynamicBuffer *exponent = malloc(sizeof(DynamicBuffer));
    DynamicBuffer *mantissa = malloc(sizeof(DynamicBuffer));

    if (exponent == NULL || mantissa == NULL)
    {
        *error_code = ERR_INTERNAL;
        return 0;
    }

    if (init_buffer(exponent, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        *error_code = ERR_INTERNAL;
        return 0;
    }

    if (init_buffer(mantissa, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        *error_code = ERR_INTERNAL;
        return 0;
    }

    int length = strlen(string);
    for (int i = 0; i < length; i++)
    {
        char c = string[i];
        if (c == 'e' || c == 'E')
        {
            // we set that we are in exponent
            exponent_started = true;
            continue;
        }

        if (c == '+' || c == '-')
        {
            continue; // we already know the sign from parameter
        }

        // from here on we are in exponent
        if (exponent_started)
        {
            if (c == '0' && skip_zeroes)
            {
                // lets skip zeroes
                continue;
            }

            if (c != '0' && skip_zeroes)
            {
                // we stop skipping zeroes
                skip_zeroes = false;
            }
            // and we append the character to exponent
            buffer_append_char(exponent, c);
        }
        else
        {

            // we are not in exponent, so we append the character to mantissa
            buffer_append_char(mantissa, c);
        }
    }

    // now we have exponent and mantissa loaded
    // lets convert them to double
    int exponent_value = (int)strtol(exponent->buffer, NULL, 10);
    if (!positive_exponent)
    {
        exponent_value *= -1;
    }

    double mantissa_value = strtod(mantissa->buffer, NULL);

    double result = mantissa_value * pow(10, exponent_value);

    free_buffer(exponent);
    free_buffer(mantissa);

    return result;
}

int get_char(FILE *source_file)
{
    return fgetc(source_file);
}


bool two_more_double_quotes(FILE *source_file)
{
    bool found = false;

    int c = get_char(source_file);
    if(c == '"'){
        int c2 = get_char(source_file);
        if(c2 == '"'){
            found = true;
        }
        ungetc(c2, source_file);
    }

    ungetc(c, source_file);

    return found;
}

/**
 * @brief Determines, if the loaded " is a middle one between two other "
*/
bool middle_double_quote(FILE *source_file, char previous_char){
    if(previous_char != '"')
        return false;

    int c = get_char(source_file);
    if(c == '"'){
        ungetc(c, source_file);
        return true;
    }
    ungetc(c, source_file);
    return false;
}


token_t get_token(FILE *source_file)
{
    token_t token;
    DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));
    DynamicBuffer *raw_buffer = malloc(sizeof(DynamicBuffer));
    if (buffer == NULL || raw_buffer == NULL)
    {
        token.type = TOKEN_ERROR;
        return token;
    }

    if (init_buffer(buffer, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        token.type = TOKEN_ERROR;
        return token;
    }
    if (init_buffer(raw_buffer, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        token.type = TOKEN_ERROR;
        return token;
    }

    token.type = TOKEN_UNKNOWN;
    token.value.int_value = 0;
    token.value.double_value = 0;
    token.value.string_value = buffer;
    token.source_value = raw_buffer;

    // we skip whitespace characters
    skip_whitespace(source_file);
    int c = get_char(source_file);

    // we encountered end of line, so we return EOL token
    if (c == '\n' && !in_block_comment_global)
    {
        token.type = TOKEN_EOL;

        return token;
        
        
    }

    // we encountered EOF, so we return EOF token
    if (c == EOF)
    {
        token.type = TOKEN_EOF;
        return token;
    }

    if (c == '-')
    {
        // arithmetic minus or arrow operator
        // determined by next character
        c = get_char(source_file);

        if (isdigit(c) || c != '>')
        {
            // arithmetic minus
            token.type = TOKEN_OPERATOR_SUB;
            buffer_append_string(raw_buffer, "-");
            ungetc(c, source_file);
            return token;
        }
        else
        {
            token.type = TOKEN_ARROW;
            buffer_append_string(raw_buffer, "->");
            return token;
        }
    }

    if (c == '"')
    {
        // this branch loads strings
        bool multiline_string = false;
        int consequitive_quotes = 0;
        bool first_new_line = true;
        // we need to check for 2 other consecutive " to determine if it is multiline string
        int potential_quote = get_char(source_file);
        if (potential_quote == '"')
        {
            int potential_quote2 = get_char(source_file);
            if (potential_quote2 == '"')
            {
                buffer_append_string(raw_buffer, "\"\"\"");
                multiline_string = true;
            }
            else
            {
                ungetc(potential_quote2, source_file);
                ungetc(potential_quote, source_file);
            }
        }
        else
        {
            ungetc(potential_quote, source_file);
        }
       

        token.type = TOKEN_STRING;


        if(!multiline_string)
            buffer_append_string(raw_buffer, "\"");


        // if we are in a multiline string, we scan till EOF (and break if we encounter 3 consecutive ")
        // if we are in a single line string, we scan till " (and break if we encounter \n)
        int till_char = multiline_string ? EOF : '"';

        int previous_char = 0;

        while ((c = get_char(source_file)) != till_char)
        {

            if(c == '"')
                consequitive_quotes++;
            else
                consequitive_quotes = 0;

            if(consequitive_quotes == 3){
                buffer_append_char(raw_buffer, c);
                return token;
            }

            if (c == EOF || (c == '\n' && !multiline_string))
            {
                // we jump out - invalid string
                token.type = TOKEN_UNKNOWN;
                buffer_clear(buffer);
                buffer_clear(raw_buffer);
                ungetc(c, source_file);
                return token;

            }else if(c == '\n' && multiline_string){
                // \n after """ and before """ is not part of the string (but its part of raw_buffer)
                // we need to skip them

                if(first_new_line){
                    buffer_append_char(raw_buffer, c);
                    first_new_line = false;
                    previous_char = c;
                    continue;
                }

                // if we encountered \n, and there is three more ", it is end of multiline string
                // therefore the \n is not part of the string (but always should be part of raw_buffer)
                int next_char = get_char(source_file);
                if(next_char == '"'){
                    if(two_more_double_quotes(source_file)){
                        buffer_append_char(raw_buffer, c);
                        ungetc(next_char, source_file);
                        previous_char = c;
                        continue;
                    }
                }
                ungetc(next_char, source_file);


                buffer_append_char(buffer, c);
                buffer_append_char(raw_buffer, c);
                previous_char = c;
                continue;
            }



            if (c == '\\')
            {
                // possible escape sequence
                c = get_char(source_file);
                if (c == EOF)
                {
                    token.type = TOKEN_UNKNOWN;
                    ungetc(c, source_file);
                    return token;
                }

                if (is_escape_sequence(c))
                {

                    buffer_append_char(raw_buffer, '\\');
                    buffer_append_char(raw_buffer, c);

                    int escape_sequence = create_escape_sequence(c);
                    if (escape_sequence == -1)
                    {
                        token.type = TOKEN_UNKNOWN;
                        free_buffer(buffer);
                        free_buffer(raw_buffer);
                        return token;
                    }

                    buffer_append_char(buffer, escape_sequence);
                }
                else if (is_unicode_escape(c))
                {
                    // now we have \u loaded
                    // need to check for { and 8 hexadecimal digits and }
                    // else throw error

                    buffer_append_char(raw_buffer, '\\');
                    buffer_append_char(raw_buffer, c);
                    c = get_char(source_file);
                    if (c == EOF)
                    {
                        token.type = TOKEN_UNKNOWN;
                        ungetc(c, source_file);
                        return token;
                    }

                    if (c == '{')
                    {
                        // now we have \u{ loaded, need to scan for up to 8 hexadecimal digits
                        const int max_hex_digits = 8;
                        int hex_digits_count = 0;
                        DynamicBuffer *hex_number = malloc(sizeof(DynamicBuffer));
                        if (init_buffer(hex_number, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
                        {
                            token.type = TOKEN_ERROR;
                            free_buffer(buffer);
                            free_buffer(raw_buffer);
                            return token;
                        }

                        buffer_append_char(raw_buffer, c);

                        while ((c = get_char(source_file)) != '}')
                        {
                            // scan for the inside of {
                            if (c == EOF)
                            {
                                token.type = TOKEN_UNKNOWN;
                                buffer_clear(buffer);
                                buffer_clear(raw_buffer);
                                ungetc(c, source_file);
                                return token;
                            }
                            // if its not a hex digit, its invalid unicode escape sequence
                            if (!isxdigit(c))
                            {
                                // invalid unicode escape sequence
                                token.type = TOKEN_UNKNOWN;
                                return token;
                            }
                            // else we append it to buffers
                            buffer_append_char(raw_buffer, c);
                            hex_digits_count++;
                            buffer_append_char(hex_number, c);

                            if (hex_digits_count > max_hex_digits)
                            {
                                // invalid unicode escape sequence

                                token.type = TOKEN_UNKNOWN;
                                return token;
                            }
                        }
                        // if there is no hex digits, it is invalid unicode escape sequence
                        if(hex_digits_count == 0){
                            token.type = TOKEN_UNKNOWN;
                            return token;
                        }


                        // now we have \u{XXXXXXXX loaded

                        buffer_append_char(raw_buffer, c);

                        // now we have \u{XXXXXXXX} loaded
                        // we need to convert to unicode character (we will use unsigned char), since only 0 - 255 will be tested
                        unsigned int unicode_number = (unsigned int)strtol(hex_number->buffer, NULL, 16);
                        unsigned char unicode_c = (unsigned char)unicode_number;
                        buffer_append_char(buffer, unicode_c);
                        free_buffer(hex_number);
                    }
                    else
                    {
                        // if there is no { after \u, it is invalid unicode escape sequence

                        token.type = TOKEN_UNKNOWN;
                        return token;
                    }

                }  // end of unicode escape sequence
                else
                {
                    // if its not escape sequence nor unicode escape sequence, its invalid
                    buffer_append_char(raw_buffer, '\\');
                    buffer_append_char(raw_buffer, c);
                    token.type = TOKEN_UNKNOWN;
                    return token;
                }
            } // end of escape sequence
            else
            {
                // its a regular character
                if(c == '"'){
                    // we need to determine if the " is part of the string or not
                    

                    if(!two_more_double_quotes(source_file) && !middle_double_quote(source_file, previous_char)){
                        // if the " does not have 2 more " after it, it is part of the string
                        // if the " is not after another ", and before another ", it is part of the string
                        buffer_append_char(buffer, c);
                        
                    }

                    buffer_append_char(raw_buffer, c);
                }else{
                    buffer_append_char(buffer, c);
                    buffer_append_char(raw_buffer, c);
                }

            }
            previous_char = c;
        }

        
       if(!multiline_string) buffer_append_string(raw_buffer, "\"");

       if(multiline_string && consequitive_quotes != 3){
            // multiline string was not ended correctly -> it is unknown token
            token.type = TOKEN_UNKNOWN;
            buffer_clear(buffer);
            buffer_clear(raw_buffer);
        }

        return token;
    }

   

    if (isdigit(c))
    {
        // this branch handles numbers
        bool is_double = false;
        bool positve_exponent = true;
        bool loading_exponent = false;
        bool sign_after_e = false;
        do
        {
            // this is definitely a double
            if (c == '.' || c == 'e' || c == 'E')
            {
                is_double = true;
            }

            // set flag that we are loading exponent
            if (c == 'e' || c == 'E')
            {
                loading_exponent = true;
            }

            if ((c == '+' || c == '-') && !loading_exponent)
            {
                // this is not a valid number, its operator between two numbers
                token.type = is_double ? TOKEN_DOUBLE : TOKEN_INT;
                ungetc(c, source_file);
                return token;
            }

            if ((c == '+' || c == '-') && sign_after_e)
            {
                // this is not a valid number, its operator between two numbers

                token.type = is_double ? TOKEN_DOUBLE : TOKEN_INT;
                ungetc(c, source_file);
                return token;
            }

            if (c == '+' || c == '-')
            {
                positve_exponent = c == '+';
                sign_after_e = true;
            }
            buffer_append_char(raw_buffer, c);

            c = get_char(source_file);

        } while (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+');

         ungetc(c, source_file);


        // lastly determine if the number is int or double
        if (is_double)
        {

            token.type = TOKEN_DOUBLE;
            // buffer now contains the whole number
            error_code_t error_code = ERR_NONE;
            double v = string_to_double(raw_buffer->buffer, positve_exponent, &error_code);

            // if the conversion was not successful, but it was internal error, we return TOKEN_ERROR
            if(error_code != ERR_NONE){
                token.type = TOKEN_ERROR;
                return token;
            }

            token.value.double_value = v;
        }
        else
        {
            token.type = TOKEN_INT;
            int v = (int)strtol(raw_buffer->buffer, NULL, 10);

            token.value.int_value = v;
        }
    }
    else if (isalpha(c) || c == '_')
    {
        // could be keyword or identifier
        // need to also check for ( and ) - function call/declaration
        buffer_clear(raw_buffer);

        // alphabet we will be loading into buffer
        while (isalnum(c) || c == '_' || c == '?')
        {
            buffer_append_char(raw_buffer, c);
            c = get_char(source_file);
        }


        // now lets check what the loaded word is
        if (word_is_keyword(raw_buffer->buffer))
        {
            token_type_t type = keyword_2_token_type(raw_buffer->buffer);
            token.type = type;
        }
        else if (word_is_identifier(raw_buffer->buffer))
        {
            token.type = TOKEN_IDENTIFIER;
        }
        else if (buffer_equals_string(raw_buffer, "_"))
        {
            // this covers the case of _ being used as a placeholder in function declaration
            token.type = TOKEN_UNDERSCORE;

        }else{

            token.type = TOKEN_UNKNOWN;
        }


        ungetc(c, source_file);
    }
    else if (c == '=')
    {
        // could be assignment or comparison
        // determine by loading next character
        c = get_char(source_file);
        if (c == '=')
        {
            token.type = TOKEN_OPERATOR_EQUAL;
            buffer_append_string(raw_buffer, "==");
        }
        else
        {
            token.type = TOKEN_OPERATOR_ASSIGN;
            buffer_append_string(raw_buffer, "=");
        }
    }
    else if (c == '!')
    {
        // could be either unary operator or comparison (not equal)
        // determine by loading next character

        c = get_char(source_file);
        if (c == '=')
        {
            token.type = TOKEN_OPERATOR_NEQ;
            buffer_append_string(raw_buffer, "!=");
        }
        else
        {
            token.type = TOKEN_OPERATOR_UNARY;
            buffer_append_string(raw_buffer, "!");
            ungetc(c, source_file);
        }
    }
    else if (c == '?')
    {
        // has to be another ? else error (since identifiers are already checked)
        // ?? - null coalescing operator
        c = get_char(source_file);
        if (c == '?')
        {
            token.type = TOKEN_OPERATOR_NIL_COALESCING;
            buffer_append_string(raw_buffer, "??");
        }
        else
        {
            token.type = TOKEN_UNKNOWN;
        }
    }
    else if (c == '/')
    {
        // could be either line comment, division or block comment
        c = get_char(source_file);

        if (c == '/')
        {
            // comment
            // skip until end of line
            if(skip_line_comment(source_file)){
                token.type = TOKEN_NONE;
            }
        }
        else if (c == '*')
        {
            /* block comment
             skip until end of block comment */
            in_block_comment_global = true;
            int result = skip_block_comment(source_file);
            if (result == EOF)
            {
                token.type = TOKEN_EOF;
            }
            else if (result == 1)
            { // 1 signals an error
                token.type = TOKEN_ERROR;
            }
            else
            {
                in_block_comment_global = false;
                token.type = TOKEN_NONE;
            }
        }
        else
        {
            // division
            ungetc(c, source_file);
            token.type = TOKEN_OPERATOR_DIV;
            buffer_append_string(buffer, "/");
        }
    }
    else if (c == '+')
    {
        // has to be addition
        token.type = TOKEN_OPERATOR_ADD;
        buffer_append_string(raw_buffer, "+");
    }
    else if (c == '*')
    {
        // has to be multiplication
        token.type = TOKEN_OPERATOR_MUL;
        buffer_append_string(raw_buffer, "*");
    }
    else if (c == '(')
    {
        // has to be left parenthesis
        token.type = TOKEN_LEFT_PARENTHESIS;
        buffer_append_string(raw_buffer, "(");
    }
    else if (c == ')')
    {
        // has to be right parenthesis
        token.type = TOKEN_RIGHT_PARENTHESIS;
        buffer_append_string(raw_buffer, ")");
    }
    else if (c == '{')
    {
        // has to be left curly bracket
        token.type = TOKEN_LEFT_BRACE;
        buffer_append_string(raw_buffer, "{");
    }
    else if (c == '}')
    {
        // has to be right curly bracket
        token.type = TOKEN_RIGHT_BRACE;
        buffer_append_string(raw_buffer, "}");
    }
    else if (c == ':')
    {
        // has to be colon
        token.type = TOKEN_COLON;
        buffer_append_string(raw_buffer, ":");
    }
    else if (c == ',')
    {
        // has to be comma
        token.type = TOKEN_COMMA;
        buffer_append_string(raw_buffer, ",");
    }
    else if (c == '<')
    {
        // could be either below or below or equal
        // determine by loading next character
        c = get_char(source_file);
        if (c == '=')
        {
            token.type = TOKEN_OPERATOR_BEQ;
            buffer_append_string(raw_buffer, "<=");
        }
        else
        {
            token.type = TOKEN_OPERATOR_BELOW;
            buffer_append_string(raw_buffer, "<");
            ungetc(c, source_file);
        }
    }
    else if (c == '>')
    {
        // could be either above or above or equal
        // determine by loading next character
        c = get_char(source_file);
        if (c == '=')
        {
            token.type = TOKEN_OPERATOR_AEQ;
            buffer_append_string(raw_buffer, ">=");
        }
        else
        {
            token.type = TOKEN_OPERATOR_ABOVE;
            buffer_append_string(raw_buffer, ">");
            ungetc(c, source_file);
        }
    }


    if(in_block_comment_global) // -- if we are still in block comment, at the end, it must be error since it was not closed
        token.type = TOKEN_UNKNOWN;

    return token;

}


/**
 * @brief Frees token
*/
void free_token(token_t token)
{
    free_buffer(token.value.string_value);
    free_buffer(token.source_value);
}


/**
 * @brief Ungets token (returns it back to source file)
*/
void unget_token(token_t token, FILE *source_file)
{

    token_type_t type = token.type;

    if (type == TOKEN_EOF)
    {
        ungetc(EOF, source_file);
        return;
    }
    if (token.type == TOKEN_EOL)
    {
        ungetc('\n', source_file);
        return;
    }

    DynamicBuffer *buffer = token.source_value;
    if (buffer == NULL)
    {
        return;
    }

    char *in_source_code = buffer->buffer;
    if (in_source_code == NULL)
    {
        return;
    }

    int length = strlen(in_source_code);
    for (int i = length - 1; i >= 0; i--)
    {
        ungetc(in_source_code[i], source_file);
    }

}

/**
 * @brief Returns token without consuming it
*/
token_t peek_token(FILE *source_file)
{
    token_t token = get_token(source_file);
    unget_token(token, source_file);
    return token;
}

#include "header_files/scanner.h"
#include "header_files/dynamic_buffer.h"

bool in_string_global = false;
bool in_block_comment_global = false;
bool in_multi_line_string_global = false;

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

const char *token_type_string_values[] = {
    "TOKEN_IDENTIFIER", // id
    "TOKEN_KEYWORD_IF", // keyword
    "TOKEN_KEYWORD_ELSE",
    "TOKEN_KEYWORD_WHILE",
    "TOKEN_KEYWORD_LET",
    "TOKEN_KEYWORD_VAR",
    "TOKEN_KEYWORD_RETURN",
    "TOKEN_KEYWORD_FUNC",

    "TOKEN_DATATYPE_INT",
    "TOKEN_DATATYPE_DOUBLE",
    "TOKEN_DATATYPE_STRING",
    "TOKEN_DATATYPE_INT_NILABLE",
    "TOKEN_DATATYPE_DOUBLE_NILABLE",
    "TOKEN_DATATYPE_STRING_NILABLE",
    "TOKEN_INT",
    "TOKEN_DOUBLE",
    "TOKEN_STRING",
    "TOKEN_NIL",

    "TOKEN_OPERATOR_ADD",            // +
    "TOKEN_OPERATOR_SUB",            // -
    "TOKEN_OPERATOR_MUL",            // *
    "TOKEN_OPERATOR_DIV",            // /
    "TOKEN_OPERATOR_BELOW",          // <
    "TOKEN_OPERATOR_ABOVE",          // >
    "TOKEN_OPERATOR_BEQ",            // <=
    "TOKEN_OPERATOR_AEQ",            // >=
    "TOKEN_OPERATOR_EQUAL",          // ==
    "TOKEN_OPERATOR_NEQ",            // !=
    "TOKEN_OPERATOR_NIL_COALESCING", // ??
    "TOKEN_OPERATOR_ASSIGN",         // =
    "TOKEN_OPERATOR_UNARY",          // !
    "TOKEN_EOL",
    "TOKEN_EOF",

    "TOKEN_LEFT_PARENTHESIS",  // (
    "TOKEN_RIGHT_PARENTHESIS", // )
    "TOKEN_LEFT_BRACE",        // {
    "TOKEN_RIGHT_BRACE",       // }
    "TOKEN_COLON",             // :
    "TOKEN_COMMA",
    "TOKEN_ARROW",
    "TOKEN_UNDERSCORE",
    "TOKEN_DOUBLE_QUOTE",
    "TOKEN_TRIPLE_DOUBLE_QUOTE",

    "TOKEN_UNKNOWN",
    "TOKEN_ERROR",
    "TOKEN_NONE",

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

bool is_escape_sequence(char c)
{
    if (c == 'n' || c == 't' || c == '"' || c == '\\' || c == '0' || c == '\'' || c == 'r')
    {
        return true;
    }

    return false;
}

int skip_block_comment(FILE *source_file)
{
    int c;
    while ((c = get_char(source_file)) != EOF)
    {
        if (c == '*')
        {
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

int skip_line_comment(FILE *source_file)
{
    int c;
    while ((c = get_char(source_file)) != EOF)
    {
        if (c == '\n')
        {
            return 0;
        }
    }

    if (c == EOF)
        return EOF;

    return 1;
}

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

bool word_is_identifier(char *word)
{
    if (isalpha(word[0]) || word[0] == '_')
    {
        if (strlen(word) == 1 && word[0] == '_')
        {
            return false;
        }
        return true;
    }

    return false;
}

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

bool is_unicode_escape(char c)
{
    return c == 'u';
}

int create_escape_sequence(char c)
{
    if (c == 'n')
    {
        return '\n';
    }
    else if (c == 't')
    {
        return '\t';
    }
    else if (c == '"')
    {
        return '"';
    }
    else if (c == '\\')
    {
        return '\\';
    }
    else if (c == '0')
    {
        return '\0';
    }
    else if (c == '\'')
    {
        return '\'';
    }
    else if (c == 'r')
    {
        return '\r';
    }

    return -1;
}

double string_to_double(char *string, bool positive_exponent)
{
    bool skip_zeroes = true;
    bool exponent_started = false;
    DynamicBuffer *exponent = malloc(sizeof(DynamicBuffer));
    DynamicBuffer *mantissa = malloc(sizeof(DynamicBuffer));

    if (exponent == NULL || mantissa == NULL)
    {
        return 0;
    }

    if (init_buffer(exponent, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        return 0;
    }

    if (init_buffer(mantissa, BUFFER_INIT_CAPACITY) != ERR_CODE_OK)
    {
        return 0;
    }

    int length = strlen(string);
    for (int i = 0; i < length; i++)
    {
        char c = string[i];
        if (c == 'e' || c == 'E')
        {
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
                continue;
            }

            if (c != '0' && skip_zeroes)
            {
                skip_zeroes = false;
            }
            buffer_append_char(exponent, c);
        }
        else
        {

            buffer_append_char(mantissa, c);
        }
    }

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


token_t get_token(FILE *source_file)
{
    token_t token;
    bool next_number_negative = false;

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

    if(!in_string_global && !in_multi_line_string_global)
        skip_whitespace(source_file);
    int c = get_char(source_file);

    if (c == '\n' && !in_block_comment_global)
    {
        
        if(!in_multi_line_string_global){
            token.type = TOKEN_EOL;
            return token;
        }else{
            buffer_append_char(raw_buffer, c);
        }
        
    }
    // TODO throw error?
    if (c == EOF)
    {
        token.type = TOKEN_EOF;
        return token;
    }

    if (c == '-')
    {
        // this branch either sets that next number will be negative or arithmetic minus
        // it could also be arrow operator

        c = get_char(source_file);
        if (isdigit(c))
        {
            next_number_negative = true;
            // ungetc(c, source_file);
        }
        else if (c != '>')
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
        // this branch either sets that string is being loaded (string literal)

        // we need to check for 2 other consecutive " to determine if it is multiline string
        int potential_quote = get_char(source_file);
        if (potential_quote == '"')
        {
            int potential_quote2 = get_char(source_file);
            if (potential_quote2 == '"')
            {
                token.type = TOKEN_TRIPLE_DOUBLE_QUOTE;
                buffer_append_string(raw_buffer, "\"\"\"");
                in_multi_line_string_global = !in_multi_line_string_global;
                return token;
            }
            else
            {
                ungetc(potential_quote2, source_file);

            }
            ungetc(potential_quote, source_file);
        }
        else
        {
            ungetc(potential_quote, source_file);
        }
       

        token.type = TOKEN_DOUBLE_QUOTE;
        buffer_append_string(raw_buffer, "\"");
        if (in_string_global)
        {
            // this is the end of string
            in_string_global = false;
            return token;
        }

        // c = get_char(source_file);
        // if (c != '"')
        // {

        //     in_string_global = true;
        //     ungetc(c, source_file);
        // }
        // else
        // {
        //     ungetc(c, source_file);
        // }

        in_string_global = true;
        return token;
    }

    if (in_string_global)
    {
        /* allowed escape sequences: \n, \t, \", \\
         */
        // todo solve escape sequences \u{XXXXXXXX}
        // raw_buffer should contain the string in source code
        // while buffer should contain the string value (meaning escape sequences should be expanded)

        buffer_clear(buffer);
        ungetc(c, source_file);

        token.type = TOKEN_STRING;

        while ((c = get_char(source_file)) != '"')
        {
            if (c == EOF)
            {
                ungetc(c, source_file);
                return token;
            }

            if (c == '\n')
            {
                // invalid string, cant throw lex error because it is syntax error
                in_string_global = false;
                ungetc(c, source_file);
                return token;
            }

            if (c == '\\')
            {
                // possible escape sequence
                c = get_char(source_file);
                if (c == EOF)
                {
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
                        token.type = TOKEN_EOF;
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
                            if (c == EOF)
                            {
                                token.type = TOKEN_EOF;
                                return token;
                            }
                            if (!isxdigit(c))
                            {
                                // invalid unicode escape sequence
                                token.type = TOKEN_UNKNOWN;
                                return token;
                            }
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

                        // now we have \u{XXXXXXXX loaded

                        buffer_append_char(raw_buffer, c);
                        // now we have \u{XXXXXXXX} loaded
                        // TODO convert to unicode character
                        unsigned int unicode_number = (unsigned int)strtol(hex_number->buffer, NULL, 16);
                        char unicode_c = (char)unicode_number;
                        buffer_append_char(buffer, unicode_c);
                        free_buffer(hex_number);
                    }
                    else
                    {

                        token.type = TOKEN_UNKNOWN;
                        return token;
                    }
                }

                else
                {

                    buffer_append_char(raw_buffer, '\\');
                    buffer_append_char(raw_buffer, c);
                    token.type = TOKEN_UNKNOWN;
                    return token;
                }
            }
            else
            {
                buffer_append_char(buffer, c);
                buffer_append_char(raw_buffer, c);

                token.type = TOKEN_STRING;
            }
        }

        ungetc(c, source_file);
    }

    if (in_multi_line_string_global)
    {
        // this branch handles multiline string
        // multiline string is ended by 3 consecutive " (""")
        // multiline string can contain any character except for 3 consecutive "

        buffer_clear(buffer);
        if(c != '\n')
            ungetc(c, source_file);

        token.type = TOKEN_STRING;

        while ((c = get_char(source_file)) != EOF)
        {
            // everytime we hit a " we need to check if it is followed by 2 other "
            // if so, its not apart of the string

           

            if (c == '"' || c == '\n')
            {
                int next_char = get_char(source_file);
                if (next_char == '"')
                {
                    int next_char2 = get_char(source_file);
                    if (next_char2 == '"')
                    {
                        // this is the end of multiline string
                        // we return all 3 " to the stream (so the next token load will recognize it as triple double quote)
                        ungetc(next_char2, source_file);
                        ungetc(next_char, source_file);

                        // there is either 3 quotes (if the initial char was ") or 2 quotes (if the initial char was \n)
                        // if the initial char was \n we need to look for one more quote to determine if the new line character shuold be included in the string

                        if(c == '"'){
                            ungetc(c, source_file);
                            return token;
                        }else {
                            int next_char3 = get_char(source_file);
                            if(next_char3 == '"'){
                                // the newline is right before 3 quotes, hence it is not part of the string
                                buffer_append_char(raw_buffer, c);
                            }

                            ungetc(next_char3, source_file);
                            return token;
                        }
                        
                    }
                    else
                    {
                        ungetc(next_char2, source_file);
                    }
                }
                else
                {
                    ungetc(next_char, source_file);
                }
                buffer_append_char(buffer, c);
                buffer_append_char(raw_buffer, c);
            }
            else
            { // not a "
                // buffer_append_char(buffer, c);
                // buffer_append_char(raw_buffer, c);

                if (c == '\\')
                {
                    // possible escape sequence
                    c = get_char(source_file);
                    if (c == EOF)
                    {
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
                            token.type = TOKEN_EOF;
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
                                if (c == EOF)
                                {
                                    token.type = TOKEN_EOF;
                                    return token;
                                }
                                if (!isxdigit(c))
                                {
                                    // invalid unicode escape sequence
                                    token.type = TOKEN_UNKNOWN;
                                    return token;
                                }
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

                            // now we have \u{XXXXXXXX loaded

                            buffer_append_char(raw_buffer, c);
                            // now we have \u{XXXXXXXX} loaded
                            // TODO convert to unicode character
                            unsigned int unicode_number = (unsigned int)strtol(hex_number->buffer, NULL, 16);
                            char unicode_c = (char)unicode_number;
                            buffer_append_char(buffer, unicode_c);
                            free_buffer(hex_number);
                        }
                        else
                        {

                            token.type = TOKEN_UNKNOWN;
                            return token;
                        }
                    }

                    else
                    {

                        buffer_append_char(raw_buffer, '\\');
                        buffer_append_char(raw_buffer, c);
                        token.type = TOKEN_UNKNOWN;
                        return token;
                    }
                }
                else
                {
                    buffer_append_char(buffer, c);
                    buffer_append_char(raw_buffer, c);

                    token.type = TOKEN_STRING;
                }
            }
        }
    }

    if (isdigit(c))
    {
        // určitě nemůže být klíčové slovo a identifikátor
        // this branch handles numbers
        bool is_double = false;
        bool positve_exponent = true;
        bool loading_exponent = false;
        bool sign_after_e = false;
        do
        {
            if (c == '.' || c == 'e' || c == 'E')
            {
                is_double = true;
            }

            if (c == 'e' || c == 'E')
            {
                loading_exponent = true;
            }

            if ((c == '+' || c == '-') && !loading_exponent)
            {
                // this is not a valid number

                token.type = TOKEN_UNKNOWN;
                return token;
            }

            if ((c == '+' || c == '-') && sign_after_e)
            {
                // this is not a valid number

                token.type = TOKEN_UNKNOWN;
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

        if (is_double)
        {

            token.type = TOKEN_DOUBLE;
            // buffer now contains the whole number
            double v = string_to_double(raw_buffer->buffer, positve_exponent);

            if (next_number_negative)
            {
                v *= -1;
                buffer_insert_char_beggining(raw_buffer, '-');
            }

            token.value.double_value = v;
        }
        else
        {
            token.type = TOKEN_INT;
            int v = (int)strtol(raw_buffer->buffer, NULL, 10);

            if (next_number_negative)
            {
                v *= -1;
                buffer_insert_char_beggining(raw_buffer, '-');
            }

            token.value.int_value = v;
        }
    }
    else if (isalpha(c) || c == '_')
    {
        // could be keyword or identifier
        // need to also check for ( and ) - function call/declaration
        buffer_clear(raw_buffer);
        while (isalnum(c) || c == '_' || c == '?')
        {
            // todo ošetřit realloc fail => err_code_internal
            buffer_append_char(raw_buffer, c);
            c = get_char(source_file);
        }

        if (word_is_keyword(raw_buffer->buffer))
        {
            token_type_t type = keyword_2_token_type(raw_buffer->buffer);
            token.type = type;
            // token.value.string_value = buffer;
        }
        else if (word_is_identifier(raw_buffer->buffer))
        {
            token.type = TOKEN_IDENTIFIER;
            // token.value.string_value = buffer;
        }
        else if (buffer_equals_string(raw_buffer, "_"))
        {
            // this covers the case of _ being used as a placeholder in function declaration
            token.type = TOKEN_UNDERSCORE;
        }
        ungetc(c, source_file);
    }
    else if (c == '=')
    {
        // could be assignment or comparison
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
            int res = skip_line_comment(source_file);
            if (res == EOF)
            {
                token.type = TOKEN_EOF;
            }
            else if (res == 1)
            {
                token.type = TOKEN_ERROR;
            }
            else
            {
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

    else
    {

        // todo throw lexical error (TOKEN_UNKNOWN is default)
    }

    return token;
}

void free_token(token_t token)
{
    free_buffer(token.value.string_value);
    free_buffer(token.source_value);
}


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

    char *string_value = buffer->buffer;
    if (string_value == NULL)
    {
        return;
    }

    int length = strlen(string_value);
    for (int i = length - 1; i >= 0; i--)
    {
        ungetc(string_value[i], source_file);
    }

}

token_t peek_token(FILE *source_file)
{
    token_t token = get_token(source_file);
    unget_token(token, source_file);
    return token;
}


// int main(void){
//     token_t token;
//     FILE *file = fopen("../test2.txt", "r");
//     if (file == NULL)
//     {
//         return 1;
//     }

//     while ((token = get_token(file)).type != TOKEN_EOF)
//     {
//         printf("%s\n", token_type_string_values[token.type]);
//         free_token(token);
//     }

//     fclose(file);
// }


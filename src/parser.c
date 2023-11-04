#include "header_files/parser.h"

TreeNode *createNewNode(TreeNode *node, error_code_t *error)
{
    TreeNode *newNode = realloc(node->children, sizeof(TreeNode) * ++node->numChildren);
    if (newNode == NULL)
    {
        *error = ERR_INTERNAL;
        return NULL;
    }
    newNode->children = NULL;
    newNode->numChildren = 0;
    return newNode;
}

void dispose(TreeNode *parseTree)
{
    while (parseTree->children != NULL)
    {
        for (unsigned i = 0; i < parseTree->numChildren; i++)
        {
            dispose(&parseTree->children[i]);
        }
    }
    free(parseTree->children);
}

void skipEmptyLines(token_t *token)
{
    *token = get_token(file);
    while (token->type == TOKEN_EOL)
    {
        *token = get_token(file);
    }
}

bool parseKeyWord(error_code_t *error)
{
    token_t token;
    if (token.type != TOKEN_IDENTIFIER)
    {
        /* code */
    }
    return false;
}

bool parseComma(TreeNode *funcParams, error_code_t *error)
{
    token_t token;
    skipEmptyLines(&token);
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
        if (token.type == TOKEN_IDENTIFIER)
        {
            return parseParameter(funcParams, error);
        }
        return parseParameter(funcParams, error);
    default:
        return false;
    }
}

bool parseParameter(TreeNode *funcParam, error_code_t *error)
{
    token_t token;
    skipEmptyLines(&token);

    TreeNode *funcParamValue = createNewNode(funcParam, error);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParamValue->terminal = true;

    skipEmptyLines(&token);

    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
        funcParamValue->type = NODE_IDENTIFIER;
        break;
    case TOKEN_STRING:
        funcParamValue->type = NODE_STRING;
        break;
    case TOKEN_INT:
        funcParamValue->type = NODE_INT;
        break;
    case TOKEN_DOUBLE:
        funcParamValue->type = NODE_DOUBLE;
        break;
    case TOKEN_NIL:
        funcParamValue->type = NODE_NIL;
        break;
    default:
        return false;
    }

    return true;
}

bool parseFuncCall(TreeNode *node, error_code_t *error)
{
    TreeNode *funcCallId = createNewNode(node, error);
    if (funcCallId == NULL)
    {
        return false;
    }
    funcCallId->type = NODE_IDENTIFIER;
    funcCallId->terminal = true;

    token_t token;
    skipEmptyLines(&token);

    TreeNode *funcParams = NULL;

    if (token.type != TOKEN_RIGHT_PARENTHESIS)
    {
        funcParams = createNewNode(node, error);
        if (funcParams == NULL)
        {
            return false;
        }
        funcParams->type = NODE_FUNCTION_PARAMS;
        funcParams->terminal = false;
    }

    while (token.type != TOKEN_RIGHT_PARENTHESIS)
    {
        if (funcParams == NULL)
        {
            return false;
        }

        TreeNode *funcParam = createNewNode(funcParams, error);
        if (funcParam == NULL)
        {
            return false;
        }
        funcParams->type = NODE_FUNCTION_PARAM;
        funcParams->terminal = false;

        TreeNode *funcParamType = createNewNode(funcParam, error);
        if (funcParamType == NULL)
        {
            return false;
        }
        funcParamType->terminal = true;

        switch (token.type)
        {
        case TOKEN_IDENTIFIER:
            funcParamType->type = NODE_IDENTIFIER;
            skipEmptyLines(&token);
            if (token.type == TOKEN_COLON && !parseParameter(funcParam, error))
            {
                return false;
            }
            break;
        case TOKEN_STRING:
            funcParamType->type = NODE_STRING;
            break;
        case TOKEN_INT:
            funcParamType->type = NODE_INT;
            break;
        case TOKEN_DOUBLE:
            funcParamType->type = NODE_DOUBLE;
            break;
        case TOKEN_NIL:
            funcParamType->type = NODE_NIL;
            break;
        default:
            return false;
        }

        skipEmptyLines(&token);
        if (token.type != TOKEN_RIGHT_PARENTHESIS && token.type != TOKEN_COMMA)
        {
            return false;
        }
    }
    return true;
}

bool parseLeftBrace(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error)
{
    if (parse(neterminal, error, true))
    {
        token_t token = get_token(file);
        skipEmptyLines(&token);

        if (token.type == TOKEN_RIGHT_BRACE)
        {
            token = get_token(file);

            if (token.type == TOKEN_EOL)
            {
                return parse(startNeterminal, error, false);
            }
        }
    }
    return false;
}

bool parseCondition(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error, bool inParenthesis)
{

    token_t token = get_token(file);
    skipEmptyLines(&token);

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        skipEmptyLines(&token);
        if (token.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(neterminal, error))
            {
                token = get_token(file);
                skipEmptyLines(&token);

                switch (token.type)
                {
                case TOKEN_RIGHT_PARENTHESIS:
                    token = get_token(file);
                    skipEmptyLines(&token);

                    if (token.type == TOKEN_LEFT_BRACE && inParenthesis)
                    {
                        return parseLeftBrace(startNeterminal, neterminal, error);
                    }
                    break;
                case TOKEN_LEFT_BRACE:
                    if (!inParenthesis)
                    {
                        return parseLeftBrace(startNeterminal, neterminal, error);
                    }
                    break;
                case TOKEN_OPERATOR_ABOVE:
                case TOKEN_OPERATOR_BELOW:
                case TOKEN_OPERATOR_MUL:
                case TOKEN_OPERATOR_DIV:
                case TOKEN_OPERATOR_ADD:
                case TOKEN_OPERATOR_SUB:
                case TOKEN_OPERATOR_BEQ:
                case TOKEN_OPERATOR_AEQ:
                case TOKEN_OPERATOR_EQUAL:
                case TOKEN_OPERATOR_NEQ:
                case TOKEN_OPERATOR_UNARY:
                    // precedenční analýza
                    break;

                default:
                    return false;
                }
            }
        }
        break;
    default:
        return false;
    }
    return false;
}

bool parseDeclaration(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error)
{
    return false;
}

bool parse(TreeNode *startNeterminal, error_code_t *error, bool innerBlock)
{
    token_t token;
    skipEmptyLines(&token);

    while (token.type != TOKEN_EOF)
    {

        TreeNode *nextNeterminal = createNewNode(startNeterminal, error);
        if (nextNeterminal == NULL)
        {
            return false;
        }

        switch (token.type)
        {
        case TOKEN_RIGHT_BRACE:
            if (innerBlock)
            {
                return true;
            }
            break;

        case TOKEN_IDENTIFIER:
            skipEmptyLines(&token);
            switch (token.type)
            {
            case TOKEN_LEFT_PARENTHESIS: // func call
                nextNeterminal->type = NODE_FUNCTION_CALL;
                if (!parseFuncCall(nextNeterminal, error))
                {
                    return false;
                }
                else
                {
                    token = get_token(file);
                    if (token.type == TOKEN_EOL)
                    {
                        return parse(startNeterminal, error, false);
                    }
                }
                break;
                /*
            case TOKEN_OPERATOR_ASSIGN: // assign
                nextNeterminal->type = NODE_ASSIGN;
                if (parseAssign(error))
                {
                    if (token.type == TOKEN_EOL)
                    {
                        return parse(startNeterminal, error, false);
                    }
                }
                break;
                */
            default:
                return false;
            }
        case TOKEN_KEYWORD_IF:
            nextNeterminal->type = NODE_IF;
            TreeNode *ifCond = createNewNode(nextNeterminal, error);

            if (ifCond == NULL)
            {
                return false;
            }
            ifCond->type = NODE_EXPRESSION;

            token = get_token(file);
            skipEmptyLines(&token);
            switch (token.type)
            {
            case TOKEN_LEFT_PARENTHESIS:
                token = get_token(file);
                skipEmptyLines(&token);

                switch (token.type)
                {
                case TOKEN_IDENTIFIER:
                case TOKEN_STRING:
                case TOKEN_INT:
                case TOKEN_DOUBLE:
                case TOKEN_NIL:
                    return parseCondition(startNeterminal, nextNeterminal, error, true);
                default:
                    return false;
                }

            case TOKEN_KEYWORD_LET:
                token = get_token(file);
                skipEmptyLines(&token);
                if (token.type == TOKEN_IDENTIFIER)
                {
                    token = get_token(file);
                    skipEmptyLines(&token);
                    if (token.type == TOKEN_LEFT_BRACE)
                    {
                        return parseLeftBrace(startNeterminal, nextNeterminal, error);
                    }
                }
                return false;

            case TOKEN_IDENTIFIER:
            case TOKEN_STRING:
            case TOKEN_INT:
            case TOKEN_DOUBLE:
            case TOKEN_NIL:
                return parseCondition(startNeterminal, nextNeterminal, error, false);
            default:
                return false;
            }

        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_VAR:
            skipEmptyLines(&token);
            if (token.type == TOKEN_IDENTIFIER)
            {
                nextNeterminal->type = NODE_ASSIGN;
                skipEmptyLines(&token);
                if (token.type == TOKEN_OPERATOR_ASSIGN)
                {
                    bool isIdentifier = false;
                    skipEmptyLines(&token);
                    switch (token.type)
                    {
                    case TOKEN_IDENTIFIER:
                    case TOKEN_STRING:
                    case TOKEN_INT:
                    case TOKEN_DOUBLE:
                    case TOKEN_NIL:
                        if (token.type == TOKEN_IDENTIFIER)
                        {
                            isIdentifier = true;
                            token = get_token(file);
                            skipEmptyLines(&token);
                            if (token.type == TOKEN_LEFT_PARENTHESIS)
                            {
                                if (parseFuncCall(nextNeterminal, error))
                                {
                                    token = get_token(file);
                                    if (token.type == TOKEN_EOL)
                                    {
                                        return parse(startNeterminal, error, false);
                                    }
                                }
                            }
                        }
                        else
                        {
                            token = get_token(file);
                            skipEmptyLines(&token);
                        }
                        switch (token.type)
                        {
                        case TOKEN_OPERATOR_UNARY:
                            if (isIdentifier)
                            {
                                // precedenční analýza
                            }
                            return false;
                        case TOKEN_OPERATOR_ADD:
                        case TOKEN_OPERATOR_SUB:
                        case TOKEN_OPERATOR_MUL:
                        case TOKEN_OPERATOR_DIV:
                            // precedenční analýza
                            break;

                        default:
                            return false;
                        }
                    default:
                        break;
                    }
                }
                /*
                if (token.type == TOKEN_COLON)
                {
                    token = get_token(file);
                    skipEmptyLines(&token);
                    if (token.type == TOKEN_DATATYPE)
                    {
                        token = get_token(file);
                        if (token.type == TOKEN_EOL)
                        {
                            skipEmptyLines(&token);
                            if (token.type == TOKEN_OPERATOR_ASSIGN)
                            {

                                if (parseAssign())
                                {
                                    token = get_token(file);
                                    if (token.type == TOKEN_EOL)
                                    {
                                        return parse(startNeterminal, error, false);
                                    }
                                }
                            }
                        }
                    }
                }
                */
                return false;
            }
            break;
        case TOKEN_UNKNOWN:
            *error = ERR_LEX_ANALYSIS;
            return false;
        case TOKEN_ERROR:
            *error = ERR_INTERNAL;
            return false;
        default:
            return false;
        }

        skipEmptyLines(&token);
    }

    return true;
}

int main()
{
    error_code_t error;
    error = ERR_SYNTAX_ANALYSIS;
    file = fopen("test.txt", "r");
    if (file == NULL)
    {
        error = ERR_INTERNAL;
        return error;
    }

    TreeNode startNeterminal;
    startNeterminal.type = NODE_PROGRAM;
    startNeterminal.numChildren = 0;
    startNeterminal.children = NULL;

    if (parse(&startNeterminal, &error, false))
    {
        error = ERR_NONE;
    }
    dispose(&startNeterminal);
    printf("%d\n", error);
    return error;
}

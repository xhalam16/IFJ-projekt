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

void emptyLines(token_t *token)
{
    while (token->type == TOKEN_EOL)
    {
        *token = get_token(stdin);
    }
}

bool parseKeyWord(error_code_t *error)
{
    token_t token;
    if (token.type != TOKEN_IDENTIFIER)
    {
        /* code */
    }
}

bool parseComma(TreeNode *funcParams, error_code_t *error)
{
    token_t token = get_token(stdin);
    emptyLines(&token);
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
        if (token.type == TOKEN_IDENTIFIER)
        {
            return parseParameter(funcParams, error, true);
        }
        return parseParameter(funcParams, error, false);
    default:
        return false;
    }
}

bool parseParameter(TreeNode *funcParams, error_code_t *error, bool isIdentifier)
{
    TreeNode *funcParam = createNewNode(funcParams, error);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParam->type = NODE_EXPRESSION;
    token_t token = get_token(stdin);
    emptyLines(&token);
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
    case TOKEN_COMMA:
        return parseComma(funcParams, error);
    case TOKEN_COLON:
        token = get_token(stdin);
        emptyLines(&token);
        switch (token.type)
        {
        case TOKEN_IDENTIFIER:
        case TOKEN_STRING:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_NIL:
            token = get_token(stdin);
            emptyLines(&token);
            if (token.type == TOKEN_COMMA)
            {
                return parseComma(funcParams, error);
            }
            if (token.type == TOKEN_RIGHT_PARENTHESIS)
            {
                return true;
            }
            return false;
        default:
            return false;
        }
        break;
    case TOKEN_LEFT_PARENTHESIS:
        if (isIdentifier)
        {
            return parseFuncCall(funcParam, error);
        }
        break;
    case TOKEN_RIGHT_PARENTHESIS:
        return true;
        break;
    default:
        return false;
    }
    return false;
}

bool parseFuncCall(TreeNode *node, error_code_t *error)
{
    TreeNode *funcCall = createNewNode(node, error);
    if (funcCall == NULL)
    {
        return false;
    }
    funcCall->type = NODE_EXPRESSION;
    TreeNode *funcId = createNewNode(funcCall, error);
    if (funcId == NULL)
    {
        return false;
    }
    funcId->type = NODE_IDENTIFIER;

    token_t token = get_token(stdin);
    emptyLines(&token);
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
        TreeNode *funcParams = createNewNode(funcCall, error);
        if (funcParams == NULL)
        {
            return false;
        }
        funcParams->type = NODE_EXPRESSION;

        if (token.type == TOKEN_IDENTIFIER)
        {
            return parseParameter(funcParams, error, true);
        }
        return parseParameter(funcParams, error, false);
    case TOKEN_RIGHT_PARENTHESIS:
        return true;
        break;
    default:
        return false;
        break;
    }
    return false;
}

bool parseLeftBrace(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error)
{
    if (parse(neterminal, error, true))
    {
        token_t token = get_token(stdin);
        emptyLines(&token);

        if (token.type == TOKEN_RIGHT_BRACE)
        {
            token = get_token(stdin);

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
    token_t token = get_token(stdin);
    emptyLines(&token);

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        token_t token = get_token(stdin);
        emptyLines(&token);
        if (token.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(neterminal, error))
            {
                token = get_token(stdin);
                emptyLines(&token);

                switch (token.type)
                {
                case TOKEN_RIGHT_PARENTHESIS:
                    token = get_token(stdin);
                    emptyLines(&token);

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
    }
    return false;
}

bool parseDeclaration(TreeNode *startNeterminal, TreeNode *neterminal, error_code_t *error)
{
    return false;
}

bool parse(TreeNode *startNeterminal, error_code_t *error, bool innerBlock)
{
    token_t token = get_token(stdin);
    emptyLines(&token);

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

    case TOKEN_EOF:
        if (!innerBlock)
        {
            return true;
        }
        break;

    case TOKEN_IDENTIFIER:
        token = get_token(stdin);
        emptyLines(&token);
        switch (token.type)
        {
        case TOKEN_LEFT_PARENTHESIS:
            nextNeterminal->type = NODE_EXPRESSION;
            if (parseFuncCall(nextNeterminal, error))
            {
                token = get_token(stdin);
                emptyLines(&token);
                if (token.type == TOKEN_EOL)
                {
                    return parse(startNeterminal, error, false);
                }
            }
            break;
        case TOKEN_OPERATOR_ASSIGN:
            nextNeterminal->type = NODE_ASSIGN;
            if (parseAssign(error))
            {
                if (token.type == TOKEN_EOL)
                {
                    return parse(startNeterminal, error, false);
                }
            }
            break;
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
        token = get_token(stdin);
        emptyLines(&token);
        switch (token.type)
        {
        case TOKEN_LEFT_PARENTHESIS:
            token = get_token(stdin);
            emptyLines(&token);

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
            token = get_token(stdin);
            emptyLines(&token);
            if (token.type == TOKEN_IDENTIFIER)
            {
                token = get_token(stdin);
                emptyLines(&token);
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
        token = get_token(stdin);
        emptyLines(&token);
        if (token.type == TOKEN_IDENTIFIER)
        {
            nextNeterminal->type = NODE_ASSIGN;
            token = get_token(stdin);
            emptyLines(&token);
            if (token.type == TOKEN_OPERATOR_ASSIGN)
            {
                token = get_token(stdin);
                emptyLines(&token);
                switch (token.type)
                {
                case TOKEN_IDENTIFIER:
                case TOKEN_STRING:
                case TOKEN_INT:
                case TOKEN_DOUBLE:
                case TOKEN_NIL:
                    bool isIdentifier = false;
                    if (token.type == TOKEN_IDENTIFIER)
                    {
                        isIdentifier = true;
                        token = get_token(stdin);
                        emptyLines(&token);
                        if (token.type == TOKEN_LEFT_PARENTHESIS)
                        {
                            if (parseFuncCall(nextNeterminal, error))
                            {
                                token = get_token(stdin);
                                if (token.type == TOKEN_EOL)
                                {
                                    return parse(startNeterminal, error, false);
                                }
                                
                            }
                        }
                    }
                    else
                    {
                        token = get_token(stdin);
                        emptyLines(&token);
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
            if (token.type == TOKEN_COLON)
            {
                token = get_token(stdin);
                emptyLines(&token);
                if (token.type == TOKEN_DATATYPE)
                {
                    token = get_token(stdin);
                    if (token.type == TOKEN_EOL)
                    {
                        emptyLines(&token);
                        if (token.type == TOKEN_OPERATOR_ASSIGN)
                        {

                            if (parseAssign())
                            {
                                token = get_token(stdin);
                                if (token.type == TOKEN_EOL)
                                {
                                    return parse(startNeterminal, error, false);
                                }
                                
                            }
                        }
                    }
                }
            }
            return false;
        }
        break;
    case TOKEN_UNKNOWN:
        *error = ERR_LEX_ANALYSIS;
        return false;
    default:
        return false;
    }

    return false;
}

int main()
{
    TreeNode startNeterminal;
    startNeterminal.type = NODE_PROGRAM;
    startNeterminal.numChildren = 0;
    startNeterminal.children = NULL;

    error_code_t error;
    error = ERR_SYNTAX_ANALYSIS;
    if (parse(&startNeterminal, &error, false))
    {
        error = ERR_NONE;
    }
    dispose(&startNeterminal);
    return error;
}

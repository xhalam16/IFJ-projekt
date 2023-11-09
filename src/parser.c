#include "header_files/parser.h"

TreeNode *createNewNode(TreeNode *node, error_code_t *error)
{
    TreeNode *newNode = realloc(node->children, sizeof(TreeNode) * ++node->numChildren);
    if (newNode == NULL)
    {
        *error = ERR_INTERNAL;
        return NULL;
    }
    return newNode;
}

void dispose(TreeNode *parseTree, unsigned numChildren)
{
    if (parseTree == NULL)
    {
        return;
    }

    for (unsigned i = 0; i < numChildren; i++)
    {
        
        dispose(parseTree->children[i], parseTree->children[i]->numChildren);
        free(parseTree->children);
        
    }

    
    /*
    while (parseTree->children != NULL)
    {
        for (unsigned i = 0; i < parseTree->numChildren; i++)
        {
            dispose(parseTree->children + i);
        }
    }
    free(parseTree->children);
    */
}

int skipEmptyLines(token_t *token, error_code_t *error)
{
    int numEols = 0;
    *token = get_token(file);
    while (token->type == TOKEN_EOL || token->type == TOKEN_NONE)
    {
        if (token->type == TOKEN_EOL)
        {
            numEols++;
        }
        *token = get_token(file);
    }
    if (token->type == TOKEN_UNKNOWN)
    {
        *error = ERR_LEX_ANALYSIS;
        return 0;
    }
    if (token->type == TOKEN_ERROR)
    {
        *error = ERR_INTERNAL;
        return 0;
    }
    return 1 + numEols;
}
/*
bool parseKeyWord(error_code_t *error)
{
    token_t token;
    if (token.type != TOKEN_IDENTIFIER)
    {

    }
    return false;
}
*/

bool parseParameters(TreeNode *funcParams, error_code_t *error)
{

    TreeNode *funcParam = createNewNode(funcParams, error);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParam->type = NODE_FUNCTION_PARAM;
    funcParam->terminal = false;

    TreeNode *funcParamValue = createNewNode(funcParam, error);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParamValue->terminal = true;

    token_t token;
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
        funcParamValue->type = NODE_IDENTIFIER;
        if (!skipEmptyLines(&token, error))
        {
            return false;
        }

        if (token.type == TOKEN_COLON)
        {
            TreeNode *funcParamRight = createNewNode(funcParam, error);
            if (funcParamRight == NULL)
            {
                return false;
            }
            funcParamRight->terminal = true;
            if (!skipEmptyLines(&token, error))
            {
                return false;
            }

            switch (token.type)
            {
            case TOKEN_IDENTIFIER:
                funcParamRight->type = NODE_IDENTIFIER;
                break;
            case TOKEN_STRING:
                funcParamRight->type = NODE_STRING;
                break;
            case TOKEN_INT:
                funcParamRight->type = NODE_INT;
                break;
            case TOKEN_DOUBLE:
                funcParamRight->type = NODE_DOUBLE;
                break;
            case TOKEN_NIL:
                funcParamRight->type = NODE_NIL;
                break;

            default:
                return false;
                break;
            }
        }
        if (token.type == TOKEN_COMMA)
        {
            return parseParameters(funcParams, error);
        }
        if (token.type == TOKEN_RIGHT_PARENTHESIS)
        {
            return true;
        }
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
    case TOKEN_RIGHT_PARENTHESIS:
        return true;
    default:
        return false;
    }
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }

    if (token.type == TOKEN_COMMA)
    {
        return parseParameters(funcParams, error);
    }
    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }
    return false;
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

    TreeNode *funcCallParams = createNewNode(node, error);
    if (funcCallParams == NULL)
    {
        return false;
    }
    funcCallId->type = NODE_FUNCTION_PARAMS;
    funcCallId->terminal = false;

    if (parseParameters(funcCallParams, error))
    {
        return true;
    }
    return false;
}

bool parseAssign(TreeNode *assign, error_code_t *error)
{
    TreeNode *assignId = createNewNode(assign, error);
    if (assignId == NULL)
    {
        return false;
    }
    assignId->type = NODE_IDENTIFIER;
    assignId->terminal = true;

    TreeNode *assignValue = createNewNode(assign, error);
    if (assignValue == NULL)
    {
        return false;
    }
    assignValue->terminal = false;

    token_t token;
    int numEols;
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }
    token_t prevToken = token;
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:

        if (!(numEols = skipEmptyLines(&token, error)))
        {
            return false;
        }

        if (token.type == TOKEN_LEFT_PARENTHESIS && prevToken.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(assignValue, error))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        switch (token.type)
        {
        case TOKEN_OPERATOR_UNARY:
            if (prevToken.type != TOKEN_IDENTIFIER)
            {
                return false;
            }
            // precedenční analýza
        case TOKEN_OPERATOR_ADD:
        case TOKEN_OPERATOR_SUB:
        case TOKEN_OPERATOR_MUL:
        case TOKEN_OPERATOR_DIV:
            // precedenční analýza
            break;
        case TOKEN_EOF:
            return true;
        default:
            return false;
        }
    default:
        return false;
        break;
    }
    return true;
}

bool parseCondition(TreeNode *neterminal, error_code_t *error, bool inParenthesis)
{

    token_t token = get_token(file);
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        if (!skipEmptyLines(&token, error))
        {
            return false;
        }

        if (token.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(neterminal, error))
            {
                token = get_token(file);
                if (!skipEmptyLines(&token, error))
                {
                    return false;
                }

                switch (token.type)
                {
                case TOKEN_RIGHT_PARENTHESIS:
                    token = get_token(file);
                    if (!skipEmptyLines(&token, error))
                    {
                        return false;
                    }

                    if (token.type == TOKEN_LEFT_BRACE && inParenthesis)
                    {
                        // return parseLeftBrace(startNeterminal, neterminal, error);
                    }
                    break;
                case TOKEN_LEFT_BRACE:
                    if (!inParenthesis)
                    {
                        // return parseLeftBrace(startNeterminal, neterminal, error);
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

bool parseDeclaration(TreeNode *neterminal, error_code_t *error)
{
    token_t token;
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }
    if (token.type != TOKEN_IDENTIFIER)
    {
        return false;
    }
    TreeNode *id = createNewNode(neterminal, error);
    if (id == NULL)
    {
        return false;
    }
    id->terminal = true;
    id->type = NODE_IDENTIFIER;
    return true;
}

bool parseIfStatement(TreeNode *node, error_code_t *error)
{
    token_t token;
    TreeNode *ifCond = createNewNode(node, error);
    if (ifCond == NULL)
    {
        return false;
    }
    ifCond->terminal = false;

    if (!skipEmptyLines(&token, error))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        ifCond->type = NODE_EXPRESSION;
        if (!skipEmptyLines(&token, error))
        {
            return false;
        }

        switch (token.type)
        {
        case TOKEN_IDENTIFIER:
        case TOKEN_STRING:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_NIL:
            if (!parseCondition(ifCond, error, true))
            {
                return false;
            }
        default:
            return false;
        }

    case TOKEN_KEYWORD_LET:
        ifCond->type = NODE_DECLARATION;

        TreeNode *let = createNewNode(node, error);
        if (let == NULL)
        {
            return false;
        }
        let->terminal = true;
        let->type = NODE_KEYWORD_LET;

        if (!parseDeclaration(ifCond, error))
        {

            return false;
        }

        break;

    case TOKEN_IDENTIFIER:
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
        ifCond->type = NODE_EXPRESSION;
        if (!parseCondition(ifCond, error, false))
        {
            return false;
        }
        break;
    default:
        return false;
    }
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }
    if (token.type != TOKEN_LEFT_BRACE)
    {
        return false;
    }
    if (!parse(node, error, true))
    {
        return false;
    }
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }
    if (token.type == TOKEN_KEYWORD_ELSE)
    {

        TreeNode *elseStatement = createNewNode(node, error);
        if (elseStatement == NULL)
        {
            return false;
        }
        elseStatement->terminal = false;
        if (!skipEmptyLines(&token, error))
        {
            return false;
        }

        if (token.type != TOKEN_LEFT_BRACE)
        {
            return false;
        }
        if (!parse(elseStatement, error, true))
        {
            return false;
        }
    }

    return true;
}

bool parse(TreeNode *startNeterminal, error_code_t *error, bool innerBlock)
{
    token_t token;
    if (!skipEmptyLines(&token, error))
    {
        return false;
    }

    while (token.type != TOKEN_EOF)
    {

        TreeNode *nextNeterminal = createNewNode(startNeterminal, error);
        if (nextNeterminal == NULL)
        {
            return false;
        }
        nextNeterminal->terminal = false;

        switch (token.type)
        {
        case TOKEN_RIGHT_BRACE:
            if (!innerBlock)
            {

                return false;
            }
            return true;

        case TOKEN_IDENTIFIER:
            if (!skipEmptyLines(&token, error))
            {
                return false;
            }

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
                    if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                    {
                        return false;
                    }
                }
                break;

            case TOKEN_OPERATOR_ASSIGN: // assign
                nextNeterminal->type = NODE_ASSIGN;
                if (!parseAssign(nextNeterminal, error))
                {
                    return false;
                }
                token = get_token(file);
                if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                {
                    return false;
                }
                break;

            default:
                return false;
            }
            break;
        case TOKEN_KEYWORD_IF:
            nextNeterminal->type = NODE_IF_STATEMENT;

            if (!parseIfStatement(nextNeterminal, error))
            {

                return false;
            }

            token = get_token(file);
            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {

                return false;
            }
            break;
        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_VAR:
            if (!skipEmptyLines(&token, error))
            {
                return false;
            }

            if (token.type == TOKEN_IDENTIFIER)
            {
                nextNeterminal->type = NODE_ASSIGN;
                if (!skipEmptyLines(&token, error))
                {
                    return false;
                }

                if (token.type == TOKEN_OPERATOR_ASSIGN)
                {
                    bool isIdentifier = false;
                    if (!skipEmptyLines(&token, error))
                    {
                        return false;
                    }

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
                            if (!skipEmptyLines(&token, error))
                            {
                                return false;
                            }

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
                            if (!skipEmptyLines(&token, error))
                            {
                                return false;
                            }
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
                    if (!skipEmptyLines(&token, error))
                    {
                        return false;
                    }

                    if (token.type == TOKEN_DATATYPE)
                    {
                        token = get_token(file);
                        if (token.type == TOKEN_EOL)
                        {
                            if (!skipEmptyLines(&token, error))
                            {
                                return false;
                            }

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

        if (!skipEmptyLines(&token, error))
        {
            return false;
        }
    }

    return true;
}

int main(void)
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
    startNeterminal.numChildren = 10;
    startNeterminal.children = NULL;
    startNeterminal.terminal = false;


    startNeterminal.children = realloc(NULL, sizeof(TreeNode *) * 10);

    for (unsigned i = 0; i < 10; i++)
    {
        startNeterminal.children[i]->numChildren = 0;
    }
    
    // TreeNode *dite = createNewNode(&startNeterminal, &error);

    // printf("%d\n", (*dite).numChildren);
    // return 0;
    /*
    if (parse(&startNeterminal, &error, false))
    {
        error = ERR_NONE;
    }
    */
    dispose(&startNeterminal, startNeterminal.numChildren);
    
    fclose(file);
    return error;
}

#include "header_files/parser.h"

bool addNode(TreeNode *parent, TreeNode *son)
{
    if (parent == NULL)
    {
        return true;
    }

    if (son == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }
    parent->children = realloc(parent->children, sizeof(TreeNode *) * ++parent->numChildren);
    if (parent->children == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }
    parent->children[parent->numChildren - 1] = son;
    return true;
}

TreeNode *createNewNode(TreeNode *parent, NodeType type, bool isTerminal)
{
    TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
    if (newNode == NULL)
    {
        error = ERR_INTERNAL;
        return NULL;
    }
    newNode->children = NULL;
    newNode->numChildren = 0;
    newNode->type = type;
    newNode->terminal = isTerminal;
    newNode->label = NULL;
    newNode->local_symtable = NULL;

    if (!addNode(parent, newNode))
    {
        free(newNode);
        return NULL;
    }
    return newNode;
}

void dispose(TreeNode *parseTree)
{
    if (parseTree == NULL)
    {
        return;
    }

    for (unsigned i = 0; i < parseTree->numChildren; i++)
    {
        dispose(parseTree->children[i]);
    }

    free(parseTree->children);

    free(parseTree);

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

int skipEmptyLines(token_t *token)
{
    int numEols = 0;
    *token = get_token(file);
    while (token->type == TOKEN_EOL || token->type == TOKEN_NONE)
    {
        if (token->type == TOKEN_EOL)
        {
            numEols++;
        }
        free_token(*token);
        *token = get_token(file);
    }
    free_token(*token);
    if (token->type == TOKEN_UNKNOWN)
    {
        error = ERR_LEX_ANALYSIS;
        return 0;
    }
    if (token->type == TOKEN_ERROR)
    {
        error = ERR_INTERNAL;
        return 0;
    }
    return 1 + numEols;
}

bool chooseDataType(TreeNode *node, token_t token)
{
    switch (token.type)
    {
    case TOKEN_DATATYPE_INT:
        node->type = NODE_DATATYPE_INT;
        break;
    case TOKEN_DATATYPE_DOUBLE:
        node->type = NODE_DATATYPE_DOUBLE;
        break;
    case TOKEN_DATATYPE_STRING:
        node->type = NODE_DATATYPE_STRING;
        break;
    case TOKEN_DATATYPE_INT_NILABLE:
        node->type = NODE_DATATYPE_INT_NILABLE;
        break;
    case TOKEN_DATATYPE_DOUBLE_NILABLE:
        node->type = NODE_DATATYPE_DOUBLE_NILABLE;
        break;
    case TOKEN_DATATYPE_STRING_NILABLE:
        node->type = NODE_DATATYPE_STRING_NILABLE;
        break;
    default:
        return false;
    }
    return true;
}

bool parseParameters(TreeNode *funcParams)
{

    TreeNode *funcParam = createNewNode(funcParams, NODE_FUNCTION_PARAM, false);
    if (funcParam == NULL)
    {
        return false;
    }

    TreeNode *funcParamValue = createNewNode(funcParam, NODE_PARAM_VALUE, false);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParamValue->terminal = true;

    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
        funcParamValue->type = NODE_IDENTIFIER;
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type == TOKEN_COLON)
        {
            TreeNode *funcParamRight = createNewNode(funcParam, NODE_PARAM_VALUE, false);
            if (funcParamRight == NULL)
            {
                return false;
            }
            funcParamRight->terminal = true;
            if (!skipEmptyLines(&token))
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
            return parseParameters(funcParams);
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
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_COMMA)
    {
        return parseParameters(funcParams);
    }
    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }
    return false;
}

bool parseFuncCall(TreeNode *node)
{

    TreeNode *funcCallId = createNewNode(node, NODE_IDENTIFIER, true);
    if (funcCallId == NULL)
    {
        return false;
    }

    TreeNode *funcCallParams = createNewNode(node, NODE_PARAM_LIST, false);
    if (funcCallParams == NULL)
    {
        return false;
    }

    if (parseParameters(funcCallParams))
    {
        return true;
    }
    return false;
}

bool parseAssign(TreeNode *assign)
{
    TreeNode *assignId = createNewNode(assign, NODE_IDENTIFIER, true);
    if (assignId == NULL)
    {
        return false;
    }

    TreeNode *assignValue = createNewNode(assign, NODE_EXPRESSION, false);
    if (assignValue == NULL)
    {
        return false;
    }

    token_t token;
    int numEols;
    if (!skipEmptyLines(&token))
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

        if (!(numEols = skipEmptyLines(&token)))
        {
            return false;
        }

        if (token.type == TOKEN_LEFT_PARENTHESIS && prevToken.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(assignValue))
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

bool parseCondition(TreeNode *neterminal, bool inParenthesis)
{

    token_t token = get_token(file);
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type == TOKEN_IDENTIFIER)
        {
            if (parseFuncCall(neterminal))
            {
                token = get_token(file);
                if (!skipEmptyLines(&token))
                {
                    return false;
                }

                switch (token.type)
                {
                case TOKEN_RIGHT_PARENTHESIS:
                    token = get_token(file);
                    if (!skipEmptyLines(&token))
                    {
                        return false;
                    }

                    if (token.type == TOKEN_LEFT_BRACE && inParenthesis)
                    {
                        // return parseLeftBrace(startNeterminal, neterminal);
                    }
                    break;
                case TOKEN_LEFT_BRACE:
                    if (!inParenthesis)
                    {
                        // return parseLeftBrace(startNeterminal, neterminal);
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

bool parseDeclaration(TreeNode *neterminal)
{
    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }
    if (token.type != TOKEN_IDENTIFIER)
    {
        return false;
    }
    TreeNode *id = createNewNode(neterminal, NODE_IDENTIFIER, true);
    if (id == NULL)
    {
        return false;
    }
    return true;
}

bool parseIfStatement(TreeNode *node)
{
    token_t token;
    TreeNode *ifCond = createNewNode(node, NODE_EXPRESSION, false);
    if (ifCond == NULL)
    {
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        ifCond->type = NODE_EXPRESSION;
        if (!skipEmptyLines(&token))
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
            if (!parseCondition(ifCond, true))
            {
                return false;
            }
        default:
            return false;
        }

    case TOKEN_KEYWORD_LET:
        ifCond->type = NODE_DECLARATION;

        TreeNode *let = createNewNode(node, NODE_KEYWORD_LET, true);
        if (let == NULL)
        {
            return false;
        }

        if (!parseDeclaration(ifCond))
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
        if (!parseCondition(ifCond, false))
        {
            return false;
        }
        break;
    default:
        return false;
    }
    if (!skipEmptyLines(&token))
    {
        return false;
    }
    if (token.type != TOKEN_LEFT_BRACE)
    {
        return false;
    }
    if (!parse(node, true))
    {
        return false;
    }
    if (!skipEmptyLines(&token))
    {
        return false;
    }
    if (token.type == TOKEN_KEYWORD_ELSE)
    {
        TreeNode *elseStatement = createNewNode(node, NODE_ELSE_STATEMENT, false);
        if (elseStatement == NULL)
        {
            return false;
        }

        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type != TOKEN_LEFT_BRACE)
        {
            return false;
        }
        if (!parse(elseStatement, true))
        {
            return false;
        }
    }

    return true;
}

bool parseParameter(TreeNode *funcParamList)
{
    TreeNode *funcParameter = createNewNode(funcParamList, NODE_FUNCTION_PARAM, false);
    if (funcParameter == NULL)
    {
        return false;
    }

    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    TreeNode *paramLabel = createNewNode(funcParameter, NODE_UNDERSCORE, true);
    if (paramLabel == NULL)
    {
        return false;
    }

    if (token.type != TOKEN_IDENTIFIER && token.type != TOKEN_UNDERSCORE)
    {
        return false;
    }
    
    if (token.type == TOKEN_IDENTIFIER)
    {
        paramLabel->type = NODE_IDENTIFIER;
        // vložení labelu parametru do tabulky symbolů
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }
  
    if (token.type != TOKEN_IDENTIFIER)
    {
        return false;
    }

    TreeNode *paramName = createNewNode(funcParameter, NODE_IDENTIFIER, true);
    if (paramName == NULL)
    {
        return false;
    }

    // vložení názvu parametru do tabulky symbolů

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type != TOKEN_COLON)
    {
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    TreeNode *paramType = createNewNode(funcParameter, NODE_INT, true);
    if (paramType == NULL)
    {
        return false;
    }

    // vložení dat. typu parametru do tabulky symbolů
    if (!chooseDataType(paramType, token))
    {
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_COMMA)
    {
        return parseParameter(funcParamList);
    }

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }
    return false;
}

bool parseParamList(TreeNode *funcParamList)
{
    token_t token;
    token = peek_token(file);

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        TreeNode *funcParameter = createNewNode(funcParamList, NODE_EPSILON, true);
        if (funcParameter == NULL)
        {
            return false;
        }
        return true;
    }
    return parseParameter(funcParamList);
}

bool parseFuncDeclaration(TreeNode *node)
{
    node->type = NODE_DECLARATION_FUNCTION;

    if (createNewNode(node, NODE_KEYWORD_FUNC, true) == NULL)
    {

        return false;
    }

    token_t token;
    if (!skipEmptyLines(&token))
    {

        return false;
    }

    if (token.type != TOKEN_IDENTIFIER)
    {

        return false;
    }

    TreeNode *funcName = createNewNode(node, NODE_IDENTIFIER, true);
    if (funcName == NULL)
    {
        return false;
    }

    // vložení názvu funkce do tabulky symbolů

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type != TOKEN_LEFT_PARENTHESIS)
    {
        return false;
    }

    TreeNode *funcParamList = createNewNode(node, NODE_PARAM_LIST, false);
    if (funcParamList == NULL)
    {
        return false;
    }

    if (!parseParamList(funcParamList))
    {
        
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_ARROW)
    {
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        TreeNode *funcReturnValue = createNewNode(node, NODE_DATATYPE_INT, true);

        // vložení dat. typu návratové hodnoty do tabulky symbolů
        if (!chooseDataType(funcReturnValue, token))
        {
            return false;
        }

        if (!skipEmptyLines(&token))
        {
            return false;
        }
    }

    if (token.type != TOKEN_LEFT_BRACE)
    {

        return false;
    }

    TreeNode *funcBody = createNewNode(node, NODE_BODY, false);

    token = peek_token(file);
    if (token.type != TOKEN_EOL)
    {
        free_token(token);
        return false;
    }
    free_token(token);

    // funcBody->local_symtable = create_local_symtable();
    
    return parse(funcBody, true);
}

bool parse(TreeNode *startNeterminal, bool inBlock)
{

    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    while (token.type != TOKEN_EOF)
    {

        TreeNode *nextNeterminal = createNewNode(startNeterminal, NODE_IDENTIFIER, false);
        if (nextNeterminal == NULL)
        {
            return false;
        }

        switch (token.type)
        {
        case TOKEN_RIGHT_BRACE:
            if (inBlock)
            {
                if (startNeterminal->numChildren == 1)
                {
                    nextNeterminal->type = NODE_EPSILON;
                    nextNeterminal->terminal = true;
                } else {
                    free(nextNeterminal);
                    startNeterminal->numChildren--;
                    startNeterminal->children[startNeterminal->numChildren] = NULL;
                }
                return true;
            }
            return false;
        case TOKEN_IDENTIFIER:
            if (!skipEmptyLines(&token))
            {
                return false;
            }

            switch (token.type)
            {
            case TOKEN_LEFT_PARENTHESIS: // func call
                nextNeterminal->type = NODE_FUNCTION_CALL;
                if (!parseFuncCall(nextNeterminal))
                {
                    return false;
                }

                token = get_token(file);
                if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                {
                    return false;
                }

                break;

            case TOKEN_OPERATOR_ASSIGN: // assign
                nextNeterminal->type = NODE_ASSIGN;
                if (!parseAssign(nextNeterminal))
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

            if (!parseIfStatement(nextNeterminal))
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
            if (!skipEmptyLines(&token))
            {
                return false;
            }

            if (token.type == TOKEN_IDENTIFIER)
            {
                nextNeterminal->type = NODE_ASSIGN;
                if (!skipEmptyLines(&token))
                {
                    return false;
                }

                if (token.type == TOKEN_OPERATOR_ASSIGN)
                {
                    bool isIdentifier = false;
                    if (!skipEmptyLines(&token))
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
                            if (!skipEmptyLines(&token))
                            {
                                return false;
                            }

                            if (token.type == TOKEN_LEFT_PARENTHESIS)
                            {
                                if (parseFuncCall(nextNeterminal))
                                {
                                    token = get_token(file);
                                    if (token.type == TOKEN_EOL)
                                    {
                                        return parse(startNeterminal, false);
                                    }
                                }
                            }
                        }
                        else
                        {
                            token = get_token(file);
                            if (!skipEmptyLines(&token))
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
                    if (!skipEmptyLines(&token))
                    {
                        return false;
                    }

                    if (token.type == TOKEN_DATATYPE)
                    {
                        token = get_token(file);
                        if (token.type == TOKEN_EOL)
                        {
                            if (!skipEmptyLines(&token))
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
                                        return parse(startNeterminal, false);
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
        case TOKEN_KEYWORD_FUNC:
            if (!parseFuncDeclaration(nextNeterminal))
            {
                return false;
            }
            token = get_token(file);
            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
                return false;
            }
            break;
        case TOKEN_UNKNOWN:
            error = ERR_LEX_ANALYSIS;
            return false;
        case TOKEN_ERROR:
            error = ERR_INTERNAL;
            return false;
        default:
            return false;
        }

        if (!skipEmptyLines(&token))
        {
            return false;
        }
    }

    if (inBlock)
    {
        return false;
    }

    return true;
}

int main(void)
{
    error = ERR_SYNTAX_ANALYSIS;
    file = fopen("test.txt", "r");
    if (file == NULL)
    {
        error = ERR_INTERNAL;
        return error;
    }

    TreeNode *startNeterminal = createNewNode(NULL, NODE_PROGRAM, false);

    if (parse(startNeterminal, false))
    {
        error = ERR_NONE;
    }

    

    dispose(startNeterminal);

    if (fclose(file) == EOF)
    {
        error = ERR_INTERNAL;
    }
    printf("%d\n", error);
    return error;
}

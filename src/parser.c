#include "header_files/parser.h"
#include "parser.h"

TreeNode tree;

TreeNode *createNewNode(TreeNode *node)
{
    TreeNode *newNode = realloc(node->children, sizeof(TreeNode) * ++node->numChildren);
    if (newNode == NULL)
    {
        return NULL;
    }
    newNode[node->numChildren - 1]->type = NULL;
    newNode[node->numChildren - 1]->children = NULL;
    newNode[node->numChildren - 1]->numChildren = 0;
    return newNode;
}

void dispose(TreeNode *node)
{
    while (node->children != NULL)
    {
        for (unsigned i = 0; i < node->numChildren; i++)
        {
            dispose(node->children[i]);
        }
    }
    free(node->children);
}

token_t emptyLines(token_t token)
{
    while (token.type == TOKEN_EOL)
    {
        token = get_token(stdin);
    }
    return token;
}

bool parseKeyWord(error_code_t *error)
{
    token_t token;
    if (token.type != TOKEN_IDENTIFIER)
    {
        /* code */
    }
}

bool parseParameter(TreeNode *node, error_code_t *error, bool assign)
{
    token_t token = get_token(stdin);
    if (token.type == TOKEN_IDENTIFIER)
    {
        token = get_token(stdin);
        switch (token.type)
        {
        case TOKEN_OPERATOR_ADD:
        
        case TOKEN_COMMA:
            return parseParameter(node, error, assign);
        case TOKEN_STRING:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_NIL:
            /* code */
            break;
        default:
            break;
        }
        
    }
    return false;
}

bool parseFuncCall(TreeNode *node, error_code_t *error)
{
    TreeNode child = createNewNode(node);
    child->type = NODE_IDENTIFIER;
    token_t token = get_token(stdin);
    switch (token->type)
    {
    case TOKEN_IDENTIFIER:
        return parseParameter(node, error, false);
        break;
    case TOKEN_RIGHT_PARENTHESIS:
        token = get_token(stdin);
        if (token.type == TOKEN_EOL || token.type == TOKEN_SEMICOLON)
        {
            return parse(error);
        }
        break;
    default:
        break;
    }
}

bool parseId(TreeNode *node, error_code_t *error, bool declaration)
{
    token_t token = get_token(stdin);
    switch (token.type)
    {
    case TOKEN_LEFT_PARENTHESIS:
        node->type = NODE_FUNCTION_CALL;
        return parseFuncCall(node, error);
    case TOKEN_OPERATOR_ASSIGN:
        node->type = NODE_ASSIGN;
        return parseAssign(error);
    default:
        return false;
    }
}

bool parse(TreeNode *node, error_code_t *error)
{
    token_t token = get_token(stdin);
    while (token.type != TOKEN_EOF)
    {
        while (token.type == TOKEN_EOL)
        {
            token = get_token(stdin);
        }
        TreeNode child = createNewNode(node);

        switch (token.type)
        {
        case TOKEN_IDENTIFIER:
            if (!parseId(child, error, false))
            {
                return false;
            }
            return parse(node, error);
        case TOKEN_KEYWORD_IF:
            token = get_token(stdin);
            if (token.type == TOKEN_LEFT_PARENTHESIS)
            {
                /* code */
            }
            break;
        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_VAR:
            token = get_token(stdin);
            if (token.type == TOKEN_IDENTIFIER)
            {
                return parseId(error, true);
            }
            break;
        }
        token_t token = get_token(stdin);
    }

    return false;
}

int main()
{
    TreeNode tree;
    tree.type = NODE_START;
    tree.numChildren = 0;
    tree.children = NULL;

    error_code_t error;
    error = ERR_SYNTAX_ANALYSIS;
    if (parse(&tree, &error))
    {
        error = ERR_NONE;
    }
    dispose(&tree);
    return error;
}

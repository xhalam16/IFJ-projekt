#include <stdio.h>
#include "parser.h"

TreeNode *createNewNode(token_t *token, int numChildren)
{
    TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
    if (newNode == NULL)
    {
        return NULL;
    }
    newNode->type = token->type;
    newNode->numChildren = numChildren;
    newNode->children = NULL;
}

void dispose(TreeNode *node)
{
    if (node == NULL)
    {
        return;
    }

    TreeNode *children = node->children;
    while (children)
    {
        /* code */
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
bool parseAssign(error_code_t *error)
{
    token_t token;

    return false;
}
bool parseId(error_code_t *error)
{
    token_t token;
    if (token.type == TOKEN_OPERATOR_ASSIGN)
    {
        return parseAssign(error);
    }
    if (token.type == TOKEN_LEFT_PARENTHESIS)
    {
        // token = nextToken;
        if (token.type == TOKEN_RIGHT_PARENTHESIS)
        {
            // token = nextToken;
        }
        if (token.type == TOKEN_UNDERSCORE)
        {
            // token = nextToken;
        }
        if (/* condition */)
        {
            /* code */
        }

        return false;
    }
    if (token.type != TOKEN_OPERATOR_ASSIGN || token.type != TOKEN_LEFT_PARENTHESIS)
    {
        return false;
    }
}

bool parse(token_t *token, error_code_t *error)
{
    token_t token;
    switch (token->type)
    {
    case TOKEN_EOF:
        return true;
    case TOKEN_EOL:
    case TOKEN_SEMICOLON:
        return parse(token, error);
    case TOKEN_UNKNOWN:
    case TOKEN_ERROR:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_STRING:
    case TOKEN_DATATYPE:
        return false;
    case TOKEN_KEYWORD:
        switch (token->value.string_value)
        {
        case "else":
        case "return":
        case "nil":
            return false;
        default:
            return parseKeyWord(error);
        }
    case TOKEN_IDENTIFIER:
        return parseId(error);
        case TOKEN_
    }
}

int main()
{
    parse();
    return 0;
}

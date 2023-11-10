#include "header_files/parser.h"

global_symtable *global_table = NULL;

NodeType token_type_to_node(token_type_t t_type){
    const Token_to_node token_to_node[] = {
        {TOKEN_DATATYPE_INT, NODE_DATATYPE_INT},
        {TOKEN_DATATYPE_DOUBLE, NODE_DATATYPE_DOUBLE},
        {TOKEN_DATATYPE_STRING, NODE_DATATYPE_STRING},
        {TOKEN_DATATYPE_INT_NILABLE, NODE_DATATYPE_INT_NILABLE},
        {TOKEN_DATATYPE_DOUBLE_NILABLE, NODE_DATATYPE_DOUBLE_NILABLE},
        {TOKEN_DATATYPE_STRING_NILABLE, NODE_DATATYPE_STRING_NILABLE}
    };

    for(int i = 0; i < sizeof(token_to_node)/sizeof(Token_to_node); i++){
        if(token_to_node[i].t_value == t_type){
            return token_to_node[i].n_value;
        }
    }
    return -1;
}

data_type_t node_type_to_data(NodeType n_type){
    const Node_to_data node_to_data[] = {
        {NODE_DATATYPE_INT, DATA_INT},
        {NODE_DATATYPE_DOUBLE, DATA_DOUBLE},
        {NODE_DATATYPE_STRING, DATA_STRING},
        {NODE_DATATYPE_INT_NILABLE, DATA_INT},
        {NODE_DATATYPE_DOUBLE_NILABLE, DATA_DOUBLE},
        {NODE_DATATYPE_STRING_NILABLE, DATA_STRING}
    };

    for(int i = 0; i < sizeof(node_to_data)/sizeof(Node_to_data); i++){
        if(node_to_data[i].n_value == n_type){
            return node_to_data[i].d_value;
        }
    }
    return -1;
}

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

    if (parseTree->label != NULL)
    {
        free(parseTree->label);
    }
    if (parseTree->local_symtable != NULL)
    {
        free(parseTree->local_symtable);
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
            strcpy(funcParam->label, token.value.string_value->buffer);
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
    node->type = NODE_FUNCTION_CALL;
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
    if (createNewNode(neterminal, NODE_IDENTIFIER, true) == NULL)
    {
        return false;
    }
    free_token(token);
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type != TOKEN_OPERATOR_ASSIGN)
    {
        return false;
    }

    if (createNewNode(neterminal, NODE_ASSIGN, true) == NULL)
    {
        return NULL;
    }

    free_token(token);
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    TreeNode *expression = createNewNode(neterminal, NODE_EXPRESSION, false);
    if (expression == NULL)
    {
        return false;
    }

    if (token.type == TOKEN_IDENTIFIER)
    {
        free_token(token);
        if (!skipEmptyLines(&token))
        {
            return false;
        }
        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {

            if (parseFuncCall(expression))
            {
                return true;
            }
        }
    }

    // precedenční analýza

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
    if (!parse(node, true, false, false))
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
        if (!parse(elseStatement, true, false, false))
        {
            return false;
        }
    }

    return true;
}

bool parseParameter(TreeNode *funcParamList, parameter_list_t *param_list)
{
    function_parameter_t *param = malloc(sizeof(function_parameter_t));
    param->next = NULL;
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
        param->label = malloc(token.source_value->size + 1);
        if (param->label == NULL)
        {
            return false;
        }

        strcpy(param->label, token.source_value->buffer);
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
    param->name = malloc(token.source_value->size + 1);
    if (param->name == NULL)
    {
        return false;
    }

    move_buffer(param->name, token.source_value);

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

    NodeType n_type = token_type_to_node(token.type);
    if(n_type == -1) return false;

    paramType->type = n_type;

    // vložení dat. typu parametru do tabulky symbolů
    NodeType param_type = paramType->type;
    data_type_t d_type = node_type_to_data(param_type);
    if(d_type == -1) return false;
    if(param_type == NODE_DATATYPE_INT_NILABLE || param_type == NODE_DATATYPE_DOUBLE_NILABLE || param_type == NODE_DATATYPE_STRING_NILABLE){
        param->nilable = true;
    }
    param->data_type = d_type;

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    parameter_list_insert(param_list, param);

    if (token.type == TOKEN_COMMA)
    {
        return parseParameter(funcParamList, param_list);
    }

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }
    return false;
}

bool parseParamList(TreeNode *funcParamList, parameter_list_t *param_list)
{
    token_t token;
    token = peek_token(file);

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        free_token(token);
        token = get_token(file);
        free_token(token);
        TreeNode *funcParameter = createNewNode(funcParamList, NODE_EPSILON, true);
        if (funcParameter == NULL)
        {
            return false;
        }

        return true;
    }

    return parseParameter(funcParamList, param_list);
}

bool parseFuncDeclaration(TreeNode *node)
{
    bool voidFunction = true;
    node->type = NODE_DECLARATION_FUNCTION;
    parameter_list_t *param_list = malloc(sizeof(parameter_list_t));
    if (param_list == NULL)
    {
        return false;
    }
    parameter_list_init(param_list);

    symtable_global_data_t *data = create_global_data(SYM_FUNC, DATA_NONE, false, true, NULL, param_list);

    char* key = NULL; 

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

    key = malloc(token.source_value->size + 1);
    if (key == NULL)
    {

        return false;
    }


    move_buffer(key, token.source_value);
    

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

    if (!parseParamList(funcParamList, param_list))
    {

        return false;
    }

    if (!skipEmptyLines(&token))
    {

        return false;
    }

    if (token.type == TOKEN_ARROW)
    {
        voidFunction = false;
        if (!skipEmptyLines(&token))
        {

            return false;
        }

        TreeNode *funcReturnValue = createNewNode(node, NODE_DATATYPE_INT, true);

        // vložení dat. typu návratové hodnoty do tabulky symbolů

        NodeType n_type = token_type_to_node(token.type);
        if(n_type == -1) return false;
        funcReturnValue->type = n_type;

        NodeType node_type = funcReturnValue->type;
        data_type_t d_type = node_type_to_data(node_type);
        if(d_type == -1) return false;
        data->data_type = d_type;


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

    int ret = symtable_insert(global_table, key, data, GLOBAL_TABLE);

    if (ret != ERR_CODE_ST_OK)
        return false;

    return parse(funcBody, true, true, voidFunction);
}

bool parseReturn(TreeNode *node, bool voidFunction)
{
    token_t token;
    token = get_token(file);

    node->type = NODE_RETURN;

    if (createNewNode(node, NODE_KEYWORD_RETURN, true) == NULL)
    {
        return false;
    }

    TreeNode *returnExpression = createNewNode(node, NODE_EXPRESSION, false);
    if (returnExpression == NULL)
    {
        return false;
    }

    if (token.type == TOKEN_EOL)
    {
        if (voidFunction)
        {
            if (createNewNode(returnExpression, NODE_EPSILON, true) == NULL)
            {
                return false;
            }
            return true;
        }
        return false;
    }

    if (token.type == TOKEN_IDENTIFIER)
    {
        free_token(token);
        if (!skipEmptyLines(&token))
        {
            return false;
        }
        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {
            free_token(token);
            if (!parseFuncCall(returnExpression))
            {
                return false;
            }
            token = get_token(file);

            if (token.type == TOKEN_EOL)
            {
                free_token(token);
                return true;
            }
            free_token(token);
            return false;
        }
    }

    // precedenční analýza
    return true;
}

bool parse(TreeNode *startNeterminal, bool inBlock, bool inFunction, bool voidFunction)
{

    if (global_table == NULL)
    {
        global_table = create_global_symtable(ST_GLOBAL_INIT_SIZE);
    }

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
                }
                else
                {
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
            nextNeterminal->type = NODE_DECLARATION;
            TreeNode *declarationType;
            if ((declarationType = createNewNode(nextNeterminal, NODE_KEYWORD_LET, true)) == NULL)
            {
                return false;
            }

            if (token.type == TOKEN_KEYWORD_VAR)
            {
                declarationType->type = NODE_KEYWORD_VAR;
            }

            if (!parseDeclaration(nextNeterminal))
            {
                return false;
            }
            token = get_token(file);
            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
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
        case TOKEN_KEYWORD_RETURN:
            free_token(token);
            if (!inFunction || !parseReturn(nextNeterminal, voidFunction))
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

void print_global_table(global_symtable *table)
{
    printf("Global table:\n");

    for(int i = 0; i < table->capacity; i++){
        symtable_record_global_t *item = table->records[i];
        if(item != NULL){
            printf("data type: %d, symbol type %d, nilable: %d, defined %d, key %s\n", item->data->data_type, item->data->symbol_type, item->data->nilable, item->data->defined,item->key);
            printf("Parameters:\n");
            parameter_list_t *list = item->data->parameters;
            for (int i = 0; i < list->size; i++)
            {
                function_parameter_t *param = list->active;
                printf("label: %s, name: %s, data type: %d, nilable: %d\n", param->label, param->name, param->data_type, param->nilable);
                parameter_list_next(list);
            }
        }
    }
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

    if (parse(startNeterminal, false, false, false))
    {
        error = ERR_NONE;
    }

    print_global_table(global_table);

    dispose(startNeterminal);

    if (fclose(file) == EOF)
    {
        error = ERR_INTERNAL;
    }
    // printf("%d\n", error);
    return error;
}

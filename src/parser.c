#include "header_files/parser.h"
#include "header_files/semantic.h"

bool parseFuncCall(TreeNode *node, DynamicBuffer *func_name);

FILE *file;
global_symtable *global_table = NULL;

static error_code_t error;
static bool inBlock = false;
static local_symtable *local_table = NULL;

Stack *stack_of_local_tables = NULL;

NodeType token_type_to_node(token_type_t t_type)
{
    const Token_to_node token_to_node[] = {
        {TOKEN_DATATYPE_INT, NODE_DATATYPE_INT},
        {TOKEN_DATATYPE_DOUBLE, NODE_DATATYPE_DOUBLE},
        {TOKEN_DATATYPE_STRING, NODE_DATATYPE_STRING},
        {TOKEN_DATATYPE_INT_NILABLE, NODE_DATATYPE_INT_NILABLE},
        {TOKEN_DATATYPE_DOUBLE_NILABLE, NODE_DATATYPE_DOUBLE_NILABLE},
        {TOKEN_DATATYPE_STRING_NILABLE, NODE_DATATYPE_STRING_NILABLE}};

    for (int i = 0; i < sizeof(token_to_node) / sizeof(Token_to_node); i++)
    {
        if (token_to_node[i].t_value == t_type)
        {
            return token_to_node[i].n_value;
        }
    }
    return -1;
}

data_type_t node_type_to_data(NodeType n_type)
{
    const Node_to_data node_to_data[] = {
        {NODE_DATATYPE_INT, DATA_INT},
        {NODE_DATATYPE_DOUBLE, DATA_DOUBLE},
        {NODE_DATATYPE_STRING, DATA_STRING},
        {NODE_DATATYPE_INT_NILABLE, DATA_INT},
        {NODE_DATATYPE_DOUBLE_NILABLE, DATA_DOUBLE},
        {NODE_DATATYPE_STRING_NILABLE, DATA_STRING},
        {NODE_INT, DATA_INT},
        {NODE_DOUBLE, DATA_DOUBLE},
        {NODE_STRING, DATA_STRING},
        {NODE_NIL, DATA_NIL}};

    for (int i = 0; i < sizeof(node_to_data) / sizeof(Node_to_data); i++)
    {
        if (node_to_data[i].n_value == n_type)
        {
            return node_to_data[i].d_value;
        }
    }
    return -1;
}

NodeType expressionTokenTypeToNode(token_type_t inputTokenType)
{
    const Token_to_node token_to_node[] = {
        {TOKEN_OPERATOR_ADD, NODE_OPERATOR_ADD},
        {TOKEN_OPERATOR_SUB, NODE_OPERATOR_SUB},
        {TOKEN_OPERATOR_MUL, NODE_OPERATOR_MUL},
        {TOKEN_OPERATOR_DIV, NODE_OPERATOR_DIV},
        {TOKEN_OPERATOR_NIL_COALESCING, NODE_OPERATOR_NIL_COALESCING},
        {TOKEN_OPERATOR_UNARY, NODE_OPERATOR_UNARY},
        {TOKEN_IDENTIFIER, NODE_IDENTIFIER},
        {TOKEN_LEFT_PARENTHESIS, NODE_LEFT_PARENTHESIS},
        {TOKEN_RIGHT_PARENTHESIS, NODE_RIGHT_PARENTHESIS},
        {TOKEN_OPERATOR_BELOW, NODE_OPERATOR_BELOW},
        {TOKEN_OPERATOR_ABOVE, NODE_OPERATOR_ABOVE}};

    NodeType inputNodeType = -1;

    for (int i = 0; i < sizeof(token_to_node) / sizeof(Token_to_node); i++)
    {
        if (token_to_node[i].t_value == inputTokenType)
        {
            inputNodeType = token_to_node[i].n_value;
        }
    }

    return inputNodeType;
}

bool node_type_nilable(NodeType nt)
{
    switch (nt)
    {
    case NODE_DATATYPE_INT_NILABLE:
    case NODE_DATATYPE_DOUBLE_NILABLE:
    case NODE_DATATYPE_STRING_NILABLE:
    case NODE_INT_NILABLE:
    case NODE_DOUBLE_NILABLE:
    case NODE_STRING_NILABLE:
        return true;
    default:
        return false;
    }
}

char nodeTypeToIndex(NodeType stackTopNodeType, NodeType inputNodeType)
{
    // Tabulka pro pravdivostní výrazy
    const char regularPrecedenceTable[][10] = {
        /*           +    -    *    /    (    )    id   !    ??   EOL  */
        /*  +   */ {'>', '>', '<', '<', '<', '>', '<', '<', '>', '>'},
        /*  -   */ {'>', '>', '<', '<', '<', '>', '<', '<', '>', '>'},
        /*  *   */ {'>', '>', '>', '>', '<', '>', '<', '<', '>', '>'},
        /*  /   */ {'>', '>', '>', '>', '<', '>', '<', '<', '>', '>'},
        /*  (   */ {'<', '<', '<', '<', '<', '=', '<', '<', '<', ' '},
        /*  )   */ {'>', '>', '>', '>', ' ', '>', ' ', '<', '>', '>'},
        /*  id  */ {'>', '>', '>', '>', ' ', '>', ' ', '>', '<', '>'},
        /*  !   */ {'>', '>', '>', '>', ' ', '>', ' ', '>', '>', '>'},
        /*  ??  */ {'<', '<', '<', '<', '<', '>', '<', '<', '>', '>'},
        /*  EOL */ {'<', '<', '<', '<', '<', ' ', '<', '<', ' ', ' '}};

    const NodeTypeToIndex nodeTypeToIndex[] = {
        {NODE_OPERATOR_ADD, 0},
        {NODE_OPERATOR_SUB, 1},
        {NODE_OPERATOR_MUL, 2},
        {NODE_OPERATOR_DIV, 3},
        {NODE_LEFT_PARENTHESIS, 4},
        {NODE_RIGHT_PARENTHESIS, 5},
        {NODE_IDENTIFIER, 6},
        {NODE_OPERATOR_UNARY, 7},
        {NODE_OPERATOR_NIL_COALESCING, 8},
        {NODE_EOL, 9}};

    int stackTopIndex = -1;
    int inputIndex = -1;

    for (int i = 0; i < sizeof(nodeTypeToIndex) / sizeof(NodeTypeToIndex); i++)
    {
        if (nodeTypeToIndex[i].n_value == stackTopNodeType)
        {
            stackTopIndex = nodeTypeToIndex[i].i_value;
        }

        if (nodeTypeToIndex[i].n_value == inputNodeType)
        {
            inputIndex = nodeTypeToIndex[i].i_value;
        }
    }

    if (stackTopIndex == -1 || inputIndex == -1)
    {
        return -1;
    }

    return regularPrecedenceTable[stackTopIndex][inputIndex];
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

void remove_terminal_child(TreeNode *node, int index)
{
    if (node == NULL)
    {
        return;
    }

    if (index >= node->numChildren)
    {
        return;
    }
    TreeNode *child = node->children[index];
    if (child == NULL)
    {
        return;
    }

    if (child->terminal)
    {
        if (child->label != NULL)
        {
            free(child->label);
        }

        free(child);
        node->children[index] = NULL;

        for (int i = index; i < node->numChildren - 1; i++)
        {
            node->children[i] = node->children[i + 1];
        }

        node->numChildren--;
        node->children = realloc(node->children, sizeof(TreeNode *) * node->numChildren);
    }
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

bool load_string(TreeNode **node, bool multi_line)
{
    // loading another token
    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    // prev_token == double quote
    // token == string (or double quote == empty string)
    // final_token == double quote or error

    if (multi_line && token.type == TOKEN_TRIPLE_DOUBLE_QUOTE)
    {
        // empty multi line string
        (*node)->type = NODE_STRING;
        return true;
    }
    else if (!multi_line && token.type == TOKEN_DOUBLE_QUOTE)
    {
        // empty string
        (*node)->type = NODE_STRING;
        return true;
    }

    token_t final_token;
    if (!skipEmptyLines(&final_token))
    {
        return false;
    }

    if (multi_line)
    {
        if (final_token.type != TOKEN_TRIPLE_DOUBLE_QUOTE)
        {
            return false;
        }
    }
    else
    {
        if (final_token.type != TOKEN_DOUBLE_QUOTE)
        {
            return false;
        }
    }

    (*node)->type = NODE_STRING;
    if (move_buffer(&(*node)->label, token.value.string_value) != ERR_CODE_OK)
    {
        return false;
    }

    return true;
}

NodeType topTerminal(Stack *stack)
{
    NodeType stackTop = -1;
    for (int i = stack->top; i != -1; i--)
    {
        stackTop = *((NodeType *)stack->frames[i].data);
        if (stackTop != NODE_EXPRESSION &&
            stackTop != NODE_LEFT_BRACE &&
            stackTop != NODE_RIGHT_BRACE)
        {
            break;
        }
    }
    return stackTop;
}

bool isRule(Stack *stack, NodeType stackTop)
{

    NodeType stackValue = *((NodeType *)(stack_top(stack)->data));
    printf("stackValue: %d\n", stackValue);
    printf("stackTop2: %d\n", stackTop);
    switch (stackTop)
    {
    case NODE_OPERATOR_UNARY:
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_EXPRESSION)
        {
            stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
            if (stackTop == NODE_OPERATOR_BELOW)
            {
                return true;
            }
        }
        break;
    case NODE_IDENTIFIER:
        
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_OPERATOR_BELOW)
        {
            return true;
        }
        break;
    case NODE_EXPRESSION:
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        switch (stackTop)
        {
        case NODE_OPERATOR_ADD:
        case NODE_OPERATOR_SUB:
        case NODE_OPERATOR_MUL:
        case NODE_OPERATOR_DIV:
        case NODE_OPERATOR_NIL_COALESCING:
            stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
            if (stackTop == NODE_EXPRESSION)
            {
                return true;
            }
            break;
        default:
            return false;
        }
        break;
    case NODE_RIGHT_PARENTHESIS:
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_EXPRESSION)
        {
            stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
            if (stackTop == NODE_LEFT_PARENTHESIS)
            {
                return true;
            }
        }
        break;
    default:
        return false;
    }
    return false;
}

bool parseExpression(TreeNode *nodeExpression)
{
    Stack *typesStack = stack_init(STACK_INIT_CAPACITY);

    NodeType input = NODE_EOL;
    Stack *stack = stack_init(STACK_INIT_CAPACITY);
    stack_push(stack, &input);
    

    token_t token;
    token = get_token(file);

    token_type_t tokenType;

    NodeType stackTop;
    char tableValue;

    do
    {
        tokenType = token.type;
        if (tokenType == TOKEN_EOF)
        {
            tokenType = TOKEN_EOL;
        }

        switch (tokenType)
        {
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_NIL:
            tokenType = TOKEN_IDENTIFIER;
            break;
        case TOKEN_DOUBLE_QUOTE:
            if (!load_string(&nodeExpression, false))
            {
                tokenType = TOKEN_IDENTIFIER;
            }
        case TOKEN_TRIPLE_DOUBLE_QUOTE:
            if (!load_string(&nodeExpression, true))
            {
                tokenType = TOKEN_IDENTIFIER;
            }
            break;
        default:
            break;
        }

        stackTop = topTerminal(stack);

        if (stackTop == -1)
        {
            printf("Error: stackTop == -1\n");
            return false;
        }

        input = expressionTokenTypeToNode(tokenType);

        printf("stackTop: %d\n", stackTop);
        printf("input: %d\n", input);

        tableValue = nodeTypeToIndex(stackTop, input);

        printf("tableValue: %c\n", tableValue);

        switch (tableValue)
        {
        case '=':
            stack_push(stack, &input);
            token = get_token(file);
            break;
        case '<':
            ;
            NodeType temp = NODE_OPERATOR_BELOW;
            stack_push(stack, &temp);
            stack_push(stack, &input);
            token = get_token(file);
            break;
        case '>':
            printf("stackTop: %d\n", stackTop);
            printf("isRule\n");
            printf("stackTop: %d\n", stackTop);
            if (isRule(stack, stackTop))
            {
                
                do
                {
                    stack_pop(stack);
                    stackTop = *((NodeType *)(stack_top(stack)->data));
                } while (stackTop != NODE_OPERATOR_BELOW);
            }
            else
            {
                printf("stackTop: %d\n", stackTop);
                return false;
            }
            break;
        default:
            return false;
        }

    } while (tokenType != TOKEN_EOL || *((NodeType *)(stack_top(stack)->data)) != NODE_EXPRESSION || stack->size != 2);
    return true;
}

bool parseParameters(TreeNode *funcParams)
{
    TreeNode *funcParam = createNewNode(funcParams, NODE_FUNCTION_PARAM, false);
    if (funcParam == NULL)
    {
        return false;
    }

    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        
        funcParam->type = NODE_EPSILON;
        funcParam->terminal = true;
        
        return true;
    }

    
    TreeNode *funcParamValue = createNewNode(funcParam, NODE_PARAM_VALUE, false);
    if (funcParam == NULL)
    {
        return false;
    }
    funcParamValue->terminal = true;

    if (move_buffer(&funcParamValue->label, token.source_value) != ERR_CODE_OK)
    {
        return false;
    }
    token_t prevToken = token;
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
        funcParamValue->type = NODE_IDENTIFIER;
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if(token.type == TOKEN_LEFT_PARENTHESIS) {
            funcParamValue->type = NODE_FUNCTION_CALL;
            funcParamValue->terminal = false;
            if(!parseFuncCall(funcParamValue, prevToken.source_value)) {
                return false;
            }
        }

        if (token.type == TOKEN_COLON)
        {

            if (move_buffer(&funcParam->label, token.source_value) != ERR_CODE_OK)
            {
                return false;
            }

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

            if (move_buffer(&funcParamRight->label, token.source_value) != ERR_CODE_OK)
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
            case TOKEN_DOUBLE_QUOTE:
                if (!load_string(&funcParamRight, false))
                {
                    return false;
                }
                break;
            case TOKEN_TRIPLE_DOUBLE_QUOTE:
                if (!load_string(&funcParamRight, true))
                {
                    return false;
                }
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

    case TOKEN_DOUBLE_QUOTE:
        if (!load_string(&funcParamValue, false))
        {
            return false;
        }
        break;

    case TOKEN_TRIPLE_DOUBLE_QUOTE:
        if (!load_string(&funcParamValue, true))
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

bool parseFuncCall(TreeNode *node, DynamicBuffer *func_name)
{
    TreeNode *funcCallId = createNewNode(node, NODE_IDENTIFIER, true);
    if (funcCallId == NULL)
    {
        return false;
    }

    if (func_name != NULL)
    {
        if (move_buffer(&funcCallId->label, func_name) != ERR_CODE_OK)
        {
            return false;
        }
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

bool parseAssign(TreeNode *assign, DynamicBuffer *id_name)
{
    
    TreeNode *assignId = createNewNode(assign, NODE_IDENTIFIER, true);
    if (assignId == NULL)
    {
        return false;
    }

    if(id_name != NULL){
        if (move_buffer(&assignId->label, id_name) != ERR_CODE_OK)
        {
            error = ERR_INTERNAL;
            return false;
        }
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
            
            assignValue->type = NODE_FUNCTION_CALL;
            if (parseFuncCall(assignValue, prevToken.source_value))
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
    
    
    /*
    if(parseExpression(assignValue)) {
        return true;
    };
    */
    return false;
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

            if (parseFuncCall(neterminal, token.source_value))
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

bool parseDeclaration(TreeNode *neterminal, bool constant)
{
    symtable_local_data_t *local_data;
    symtable_global_data_t *global_data;

    // create local symtable if there is not currently one
    bool push = false;
    if(local_table == NULL){
        local_table = create_local_symtable(ST_LOCAL_INIT_SIZE);

        if(local_table == NULL){
            error = ERR_INTERNAL;
            return false;
        }

        push = true;
    }

    symbol_type_t type = constant ? SYM_CONSTANT : SYM_VAR;

    if (inBlock)
    {
        local_data = create_local_data(type, DATA_NONE, false, false, NULL);
    }
    else
    {
        global_data = create_global_data(type, DATA_NONE, false, false, NULL, NULL);
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

    // store identifier to symtable
    char* key = NULL;
    if(move_buffer(&key, token.source_value) != ERR_CODE_OK){
        error = ERR_INTERNAL;
        return false;
    }


    free_token(token);

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type != TOKEN_OPERATOR_ASSIGN && token.type != TOKEN_COLON)
    {
        return false;
    }

    data_type_t dataType = -1;
    NodeType nodeType = -1;

    if (token.type == TOKEN_COLON)
    {
        free_token(token);
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token_type_to_node(token.type) == -1)
        {
            return false;
        }

        nodeType = token_type_to_node(token.type);
        dataType = node_type_to_data(nodeType);

        if (dataType == -1)
        {
            return false;
        }

        

        if (inBlock)
        {
            local_data->data_type = dataType;
            local_data->nilable = node_type_nilable(nodeType);
        }
        else
        {
            global_data->data_type = dataType;
            global_data->nilable = node_type_nilable(nodeType);
        }

        free_token(token);
        token = get_token(file);

        if (token.type != TOKEN_OPERATOR_ASSIGN && (token.type == TOKEN_EOL || (token.type == TOKEN_EOF && !inBlock)))
        {
            if (createNewNode(neterminal, NODE_IDENTIFIER, true) == NULL)
            {
                return false;
            }
        

            if (createNewNode(neterminal, nodeType, true) == NULL)
            {
                return false;
            }

            if(inBlock){
                if(symtable_search(local_table, key, LOCAL_TABLE) != NULL){
                    error = ERR_SEMANTIC_DEFINITION;
                    return false;
                }

                if(symtable_insert(local_table, key, local_data, LOCAL_TABLE) != ERR_CODE_OK){
                    error = ERR_INTERNAL;
                    return false;
                }

            }else{
                if(symtable_search(global_table, key, GLOBAL_TABLE) != NULL){
                    error = ERR_SEMANTIC_DEFINITION;
                    return false;
                }

                if(symtable_insert(global_table, key, global_data, GLOBAL_TABLE) != ERR_CODE_OK){
                    error = ERR_INTERNAL;
                    return false;
                }

            }

            return true;
        }
    }

    // store identifier into symtable
    
    if (token.type != TOKEN_OPERATOR_ASSIGN)
    {
        free_token(token);
        return false;
    }

    if (inBlock)
    {
        local_data->defined = true;
    }
    else
    {
        global_data->defined = true;
    }

    // store identifier into symtable
    if(inBlock){
        if(symtable_search(local_table, key, LOCAL_TABLE) != NULL){
            error = ERR_SEMANTIC_DEFINITION;
            return false;
        }

        if(symtable_insert(local_table, key, local_data, LOCAL_TABLE) != ERR_CODE_OK){
            error = ERR_INTERNAL;
            return false;
        }

    }else{
        if(symtable_search(global_table, key, GLOBAL_TABLE) != NULL){
            error = ERR_SEMANTIC_DEFINITION;
            return false;
        }

        if(symtable_insert(global_table, key, global_data, GLOBAL_TABLE) != ERR_CODE_OK){
            error = ERR_INTERNAL;
            return false;
        }

    }




    neterminal->type = NODE_ASSIGN;

    free_token(token);
    TreeNode *declaration = createNewNode(neterminal, NODE_DECLARATION, false);
    if (declaration == NULL)
    {
        return false;
    }

    if (createNewNode(declaration, NODE_IDENTIFIER, true) == NULL)
    {
        return false;
    }

    declaration->children[0]->label = key;


    TreeNode *dataTypeNode = createNewNode(declaration, NODE_EPSILON, true);
    if (dataTypeNode == NULL)
    {
        return false;
    }
    if (nodeType != -1)
    {
        dataTypeNode->type = nodeType;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    TreeNode *expression = createNewNode(neterminal, NODE_EXPRESSION, false);
    if (expression == NULL)
    {
        return false;
    }

    if(push && inBlock){
        if(stack_push(stack_of_local_tables, local_table) != STACK_SUCCESS){
            error = ERR_INTERNAL;
            return false;
        }
    }
   

    if (token.type == TOKEN_IDENTIFIER)
    {
        DynamicBuffer *b = token.source_value;
        if (!skipEmptyLines(&token))
        {
            free_token(token);
            return false;
        }



        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {

            free_token(token);

            expression->type = NODE_FUNCTION_CALL;

            if (parseFuncCall(expression, b))
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

        if (!parseDeclaration(ifCond, true))
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
    inBlock = true;
    if (!parse(node, false, false))
    {
        return false;
    }
    inBlock = false;
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
        inBlock = true;
        if (!parse(elseStatement, false, false))
        {
            return false;
        }
        inBlock = false;
    }

    return true;
}

bool parseParameter(TreeNode *funcParamList, parameter_list_t *param_list)
{
    function_parameter_t *param = malloc(sizeof(function_parameter_t));

    if (param == NULL)
    {

        return false;
    }

    init_param(param);

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
        if (move_buffer(&param->label, token.source_value) != ERR_CODE_OK)
        {
            return false;
        }
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
    if (move_buffer(&param->name, token.source_value) != ERR_CODE_OK)
    {
        return false;
    }

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
    if (n_type == -1)
        return false;

    paramType->type = n_type;

    // vložení dat. typu parametru do tabulky symbolů
    NodeType param_type = paramType->type;
    data_type_t d_type = node_type_to_data(param_type);
    if (d_type == -1)
        return false;
    if (param_type == NODE_DATATYPE_INT_NILABLE || param_type == NODE_DATATYPE_DOUBLE_NILABLE || param_type == NODE_DATATYPE_STRING_NILABLE)
    {
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

    // defined - depending if the function has a body
    // into value we store a bool if the function has been checked by semantic (initialized to false)
    bool checked_by_semantic = false;
    symtable_global_data_t *data = create_global_data(SYM_FUNC, DATA_NONE, false, false, &checked_by_semantic, param_list);

    char *key = NULL;

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
    DynamicBuffer *bf = token.source_value;

    if (move_buffer(&funcName->label, bf) != ERR_CODE_OK)
    {
        return false;
    }
    // vložení názvu funkce do tabulky symbolů

    if (move_buffer(&key, bf) != ERR_CODE_OK)
    {
        return false;
    }

    free_token(token);

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

    free_token(token);
    // if (!skipEmptyLines(&token))
    // {

    //     return false;
    // }
    token = get_token(file);

    TreeNode *funcReturnValue = createNewNode(node, NODE_DATATYPE_INT, true);

    if (funcReturnValue == NULL)
    {
        return false;
    }

    if (token.type == TOKEN_ARROW)
    {

        voidFunction = false;
        free_token(token);
        if (!skipEmptyLines(&token))
        {

            return false;
        }

        // vložení dat. typu návratové hodnoty do tabulky symbolů

        NodeType n_type = token_type_to_node(token.type);
        if (n_type == -1)
            return false;
        funcReturnValue->type = n_type;

        NodeType node_type = funcReturnValue->type;
        data_type_t d_type = node_type_to_data(node_type);
        if (d_type == -1)
            return false;

        data->nilable = node_type_nilable(node_type);
        data->data_type = d_type;

        free_token(token);
        if (!skipEmptyLines(&token))
        {
            return false;
        }
    }
    else
    {
        funcReturnValue->type = NODE_EPSILON;
    }

    bool l_brace = false;
    if (token.type == TOKEN_LEFT_BRACE)
    {
        data->defined = true;
        l_brace = true;
    }

       

    token = peek_token(file);
    if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
    {
        free_token(token);
        return false;
    }

    free_token(token);
    // if (funcBody->local_symtable == NULL)
    // {
    //     funcBody->local_symtable = create_local_symtable(ST_LOCAL_INIT_SIZE);

    //     // malloc failed
    //     if (funcBody->local_symtable == NULL)
    //     {

    //         return false;
    //     }
    // }

    int ret = symtable_insert(global_table, key, data, GLOBAL_TABLE);

    if (ret != ERR_CODE_ST_OK)
        return false;

    if (l_brace)
    {
         TreeNode *funcBody = createNewNode(node, NODE_BODY, false);
        inBlock = true;
        if (parse(funcBody, true, voidFunction))
        {
            inBlock = false;
            return true;
        }
    }else{
        if(!createNewNode(node, NODE_EPSILON, true)) {
            return false;
        }
    }

    return true;
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

    TreeNode *returnExpressionValue = createNewNode(returnExpression, NODE_EPSILON, true);
    if (returnExpressionValue == NULL)
    {
        return false;
    }

    if (token.type == TOKEN_EOL)
    {
        if (voidFunction)
        {

            return true;
        }
        return false;
    }

    if (token.type == TOKEN_IDENTIFIER)
    {
        DynamicBuffer *buff = token.source_value;
        // free_token(token);
        if (!skipEmptyLines(&token))
        {
            return false;
        }
        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {
            free_token(token);

            if (returnExpression->children[0]->type == NODE_EPSILON)
            {
                // need to delete the epsilon node
                remove_terminal_child(returnExpression, 0);
            }

            if (!parseFuncCall(returnExpression, buff))
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
        returnExpressionValue->type = NODE_IDENTIFIER;
    }
    else
    {
        NodeType n_type = token_type_to_node(token.type);
        if (n_type == -1)
            return false;

        returnExpressionValue->type = n_type;
    }

    free_token(token);
    // precedenční analýza
    return true;
}

bool parse(TreeNode *startNeterminal, bool inFunction, bool voidFunction)
{
    error_code_t semantic_result;

    if (global_table == NULL)
    {
        global_table = create_global_symtable(ST_GLOBAL_INIT_SIZE);
        // malloc failed
        if (global_table == NULL)
        {
            return false;
        }
    }
    else
    {
        // local_symtable *local_table = create_local_symtable(ST_LOCAL_INIT_SIZE);
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
        printf("token type: %d\n", token.type);
        switch (token.type)
        {
        case TOKEN_RIGHT_BRACE:
            if (inBlock)
            {
                nextNeterminal->type = NODE_BODY_END;
                nextNeterminal->terminal = true;

               

                
                // if (startNeterminal->numChildren == 1)
                // {
                //     nextNeterminal->type = NODE_BODY_END;
                //     nextNeterminal->terminal = true;
                // }
                // else
                // {
                //     if (createNewNode(nextNeterminal, NODE_BODY_END, true) == NULL)
                //     {
                //         return false;
                //     }
                //     // free(nextNeterminal);
                //     // startNeterminal->numChildren--;
                //     // startNeterminal->children[startNeterminal->numChildren] = NULL;
                // }
                stack_pop(stack_of_local_tables); // pop local table (since we are leaving the block)
                local_table = NULL; // set local table to NULL (since we are leaving the block)
                return true;
            }
            return false;
        case TOKEN_IDENTIFIER:
            ;
            DynamicBuffer *buff_copy = token.source_value;
            if (!skipEmptyLines(&token))
            {
                return false;
            }

            switch (token.type)
            {
            case TOKEN_LEFT_PARENTHESIS: // func call
                
                nextNeterminal->type = NODE_FUNCTION_CALL;
                if (!parseFuncCall(nextNeterminal, buff_copy))
                {
                    return false;
                }

                semantic_result= semantic(nextNeterminal);
                printf("semantic result func_call: %d\n", semantic_result);
                if(semantic_result != ERR_NONE){
                    error = semantic_result;
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
                if (!parseAssign(nextNeterminal, buff_copy))
                {
                    return false;
                }

                semantic_result = semantic(nextNeterminal);
                printf("semantic result assign: %d\n", semantic_result);
                if(semantic_result != ERR_NONE){
                    error = semantic_result;
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
            ;
            bool constant = token.type == TOKEN_KEYWORD_LET ? true : false;


            nextNeterminal->type = NODE_DECLARATION;
            if (!parseDeclaration(nextNeterminal, constant))
            {

                return false;
            }


            free_token(token);

            break;
        case TOKEN_KEYWORD_FUNC:
            if (inBlock == true || !parseFuncDeclaration(nextNeterminal))
            {
         
                return false;
            }

            semantic_result = semantic(nextNeterminal);
            printf("semantic result func_decl: %d\n", semantic_result);
            if(semantic_result != ERR_NONE){
                error = semantic_result;
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
    printf("Global table:");
    if(get_size(table) == 0){
        printf(" is empty\n");
        return;
    }

    printf("\n");
    for (int i = 0; i < table->capacity; i++)
    {
        symtable_record_global_t *item = table->records[i];
        if (item != NULL)
        {
            printf("data type: %d, symbol type %d, nilable: %d, defined %d, key %s\n", item->data->data_type, item->data->symbol_type, item->data->nilable, item->data->defined, item->key);
            printf("Parameters:\n");
            parameter_list_t *list = item->data->parameters;
            if (list == NULL)
                continue;
            for (int i = 0; i < list->size; i++)
            {
                function_parameter_t *param = list->active;
                printf("label: %s, name: %s, data type: %d, nilable: %d\n", param->label, param->name, param->data_type, param->nilable);
                parameter_list_next(list);
            }
        }
    }
}

void print_local_table(local_symtable *table)
{
    printf("Local table:\n");

    for (int i = 0; i < table->capacity; i++)
    {
        symtable_record_local_t *item = table->records[i];
        if (item != NULL)
        {
            printf("data type: %d, symbol type %d, nilable: %d, defined %d, key %s\n", item->data->data_type, item->data->symbol_type, item->data->nilable, item->data->defined, item->key);
        }
    }
}


char *node_type_to_string(NodeType n)
{

    char *translate[] = {
    "NODE_PROGRAM",
    "NODE_BODY",
    "NODE_BODY_END",
    "NODE_ASSIGN",
    "NODE_DECLARATION",
    "NODE_DECLARATION_FUNCTION",
    "NODE_EXPRESSION",
    "NODE_IF_STATEMENT",
    "NODE_ELSE_STATEMENT",
    "NODE_RETURN",
    "NODE_KEYWORD_LET",
    "NODE_KEYWORD_VAR",
    "NODE_KEYWORD_RETURN",
    "NODE_KEYWORD_FUNC",
    "NODE_FUNCTION_CALL",
    "NODE_FUNCTION_PARAM",
    "NODE_PARAM_VALUE",
    "NODE_PARAM_LIST",
    "NODE_IDENTIFIER",
    "NODE_INT",
    "NODE_DOUBLE",
    "NODE_STRING",
    "NODE_NIL",
    "NODE_INT_NILABLE",
    "NODE_DOUBLE_NILABLE",
    "NODE_STRING_NILABLE",
    "NODE_OPERATOR_ADD",
    "NODE_OPERATOR_SUB",
    "NODE_OPERATOR_MUL",
    "NODE_OPERATOR_DIV",
    "NODE_OPERATOR_BELOW",
    "NODE_OPERATOR_ABOVE",
    "NODE_OPERATOR_BEQ",
    "NODE_OPERATOR_AEQ",
    "NODE_OPERATOR_EQUAL",
    "NODE_OPERATOR_NEQ",
    "NODE_OPERATOR_NIL_COALESCING",
    "NODE_OPERATOR_UNARY",
    "NODE_LEFT_PARENTHESIS",
    "NODE_RIGHT_PARENTHESIS",
    "NODE_RIGHT_BRACE",
    "NODE_LEFT_BRACE",
    "NODE_EOL",
    "NODE_DATATYPE_INT",
    "NODE_DATATYPE_DOUBLE",
    "NODE_DATATYPE_STRING",
    "NODE_DATATYPE_INT_NILABLE",
    "NODE_DATATYPE_DOUBLE_NILABLE",
    "NODE_DATATYPE_STRING_NILABLE",
    "NODE_EPSILON",
    "NODE_UNDERSCORE"
    };
    return translate[n];
}

void printTree(TreeNode *root)
{

    if (root == NULL)
    {
        return;
    }

    printf("%s, with value %s\n", node_type_to_string(root->type), root->label);

    for (unsigned i = 0; i < root->numChildren; i++)
    {
        printf("child %d of node %s = ", i, node_type_to_string(root->type));
        printTree(root->children[i]);
    }
}

void print_stack(Stack *stack)
{
    printf("Stack:\n");
    for (int i = 0; i <  stack_size(stack); i++)
    {
        Stack_Frame *frame = stack_get(stack, i);
        print_local_table(frame->data);

    }
}

int main(void)
{

    stack_of_local_tables = stack_init(STACK_INIT_CAPACITY);
    if(stack_of_local_tables == NULL){
        error = ERR_INTERNAL;
        return error;
    }

    error = ERR_SYNTAX_ANALYSIS;
    file = fopen("test.txt", "r");
    if (file == NULL)
    {
        error = ERR_INTERNAL;
        return error;
    }

    TreeNode *startNeterminal = createNewNode(NULL, NODE_PROGRAM, false);

    if (parse(startNeterminal, false, false))
    {
        
        error = ERR_NONE;
    }
    

    print_global_table(global_table);
    print_stack(stack_of_local_tables);
    printTree(startNeterminal);

    dispose(startNeterminal);

    if (fclose(file) == EOF)
    {
        error = ERR_INTERNAL;
    }

    printf("%d\n", error);
    return error;
}

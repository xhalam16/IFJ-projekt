/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: parser.c
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 */


#include "header_files/parser.h"
#include "header_files/semantic.h"
#include "header_files/code_gen.h"

bool parseFuncCall(TreeNode *node, DynamicBuffer *func_name);

FILE *file;
global_symtable *global_table = NULL;

static error_code_t error;
static unsigned inBlock = 0;
static bool inFunction = false;
static local_symtable *local_table = NULL;

Stack *stack_of_local_tables = NULL;

bool fill_global_builtin_functions(global_symtable *table)
{
    symtable_global_data_t *readInt = create_global_data(SYM_FUNC, DATA_INT, true, false, NULL, NULL);       // readInt
    symtable_global_data_t *readDouble = create_global_data(SYM_FUNC, DATA_DOUBLE, true, false, NULL, NULL); // readDouble
    symtable_global_data_t *readString = create_global_data(SYM_FUNC, DATA_STRING, true, false, NULL, NULL); // readString

    // int2double
    // param: 1 - _ term : Int
    // return: Double
    parameter_list_t *int2double_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param = malloc(sizeof(function_parameter_t));
    if (int2double_params == NULL || param == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(int2double_params);
    init_param(param);
    param->data_type = DATA_INT;
    param->name = "term";

    parameter_list_insert(int2double_params, param);
    symtable_global_data_t *int2double = create_global_data(SYM_FUNC, DATA_DOUBLE, false, false, NULL, int2double_params); // int2double

    // double2int
    // param: 1 - _ term : Double
    // return: Int

    parameter_list_t *double2int_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param2 = malloc(sizeof(function_parameter_t));
    if (double2int_params == NULL || param2 == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(double2int_params);
    init_param(param2);
    param2->data_type = DATA_DOUBLE;
    param2->name = "term";
    parameter_list_insert(double2int_params, param2);
    symtable_global_data_t *double2int = create_global_data(SYM_FUNC, DATA_INT, false, false, NULL, double2int_params); // double2int

    // length
    // param: 1 - _ s : String
    // return: Int
    parameter_list_t *length_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param3 = malloc(sizeof(function_parameter_t));

    if (length_params == NULL || param3 == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(length_params);
    init_param(param3);
    param3->data_type = DATA_STRING;
    param3->name = "s";
    parameter_list_insert(length_params, param3);
    symtable_global_data_t *length = create_global_data(SYM_FUNC, DATA_INT, false, false, NULL, length_params); // length

    // substring
    // param: 1 - of s : String
    // param: 2 - startingAt i : Int
    // param: 3 - endingBefore j : Int
    // return: String?

    parameter_list_t *substring_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param4 = malloc(sizeof(function_parameter_t));
    function_parameter_t *param5 = malloc(sizeof(function_parameter_t));
    function_parameter_t *param6 = malloc(sizeof(function_parameter_t));

    if (substring_params == NULL || param4 == NULL || param5 == NULL || param6 == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(substring_params);

    init_param(param4);
    param4->data_type = DATA_STRING;
    param4->label = "of";
    param4->name = "s";
    parameter_list_insert(substring_params, param4);

    init_param(param5);
    param5->data_type = DATA_INT;
    param5->label = "startingAt";
    param5->name = "i";
    parameter_list_insert(substring_params, param5);

    init_param(param6);
    param6->data_type = DATA_INT;
    param6->label = "endingBefore";
    param6->name = "j";
    parameter_list_insert(substring_params, param6);

    symtable_global_data_t *substring = create_global_data(SYM_FUNC, DATA_STRING, true, false, NULL, substring_params); // substring

    // ord
    // param: 1 - _ c : String
    // returns Int

    parameter_list_t *ord_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param7 = malloc(sizeof(function_parameter_t));

    if (ord_params == NULL || param7 == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(ord_params);
    init_param(param7);
    param7->data_type = DATA_STRING;
    param7->name = "s";
    parameter_list_insert(ord_params, param7);
    symtable_global_data_t *ord = create_global_data(SYM_FUNC, DATA_INT, false, false, NULL, ord_params); // ord

    // chr
    // param: 1 - _ i : Int
    // returns String

    parameter_list_t *chr_params = malloc(sizeof(parameter_list_t));
    function_parameter_t *param8 = malloc(sizeof(function_parameter_t));

    if (chr_params == NULL || param8 == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(chr_params);
    init_param(param8);
    param8->data_type = DATA_INT;
    param8->name = "i";
    parameter_list_insert(chr_params, param8);
    symtable_global_data_t *chr = create_global_data(SYM_FUNC, DATA_STRING, false, false, NULL, chr_params); // chr

    // todo: write inifinite number of parameters
    parameter_list_t *inputs_params = malloc(sizeof(parameter_list_t));

    if (inputs_params == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    parameter_list_init(inputs_params);
    inputs_params->size = SIZE_MAX;                                                                             // this signals that the function can have infinite number of parameters
    symtable_global_data_t *write = create_global_data(SYM_FUNC, DATA_NONE, false, false, NULL, inputs_params); // write

    symtable_global_data_t *inputs_arr[] = {
        readInt,
        readDouble,
        readString,
        int2double,
        double2int,
        length,
        substring,
        ord,
        chr,
        write};

    char *keys_built_in[] = {
        "readInt",
        "readDouble",
        "readString",
        "Int2Double",
        "Double2Int",
        "length",
        "substring",
        "ord",
        "chr",
        "write"};

    for (int i = 0; i < sizeof(inputs_arr) / sizeof(inputs_arr[0]); i++)
    {
        if (inputs_arr[i] == NULL)
        {
            return false;
        }

        if (symtable_insert(table, keys_built_in[i], inputs_arr[i], GLOBAL_TABLE) != ERR_CODE_ST_OK)
        {

            return false;
        }
    }

    return true;
}

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
        {TOKEN_OPERATOR_ABOVE, NODE_OPERATOR_ABOVE},
        {TOKEN_OPERATOR_EQUAL, NODE_OPERATOR_EQUAL},
        {TOKEN_OPERATOR_BEQ, NODE_OPERATOR_BEQ},
        {TOKEN_OPERATOR_AEQ, NODE_OPERATOR_AEQ},
        {TOKEN_OPERATOR_NEQ, NODE_OPERATOR_NEQ},
        {TOKEN_EOL, NODE_EOL}};

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
    const char regularPrecedenceTable[][9] = {
        /*            2    1    (    )    id   0    4    END  3   */
        /*  2    */ {'>', '<', '<', '>', '<', '<', '>', '>', '>'},
        /*  1    */ {'>', '>', '<', '>', '<', '<', '>', '>', '>'},
        /*  (    */ {'<', '<', '<', '=', '<', '<', '<', ' ', '<'},
        /*  )    */ {'>', '>', ' ', '>', ' ', '>', '>', '>', '>'},
        /*  id   */ {'>', '>', ' ', '>', ' ', '>', '>', '>', '>'},
        /*  0    */ {'>', '>', ' ', '>', ' ', '>', '>', '>', '>'},
        /*  4    */ {'<', '<', '<', '>', '<', '<', '<', '>', '<'},
        /*  END  */ {'<', '<', '<', ' ', '<', '<', '<', ' ', '<'},
        /*  3    */ {'<', '<', '<', '>', '<', '<', '>', '>', '>'}};

    const NodeTypeToIndex nodeTypeToIndex[] = {
        {NODE_OPERATOR_ADD, 0},
        {NODE_OPERATOR_SUB, 0},
        {NODE_OPERATOR_MUL, 1},
        {NODE_OPERATOR_DIV, 1},
        {NODE_LEFT_PARENTHESIS, 2},
        {NODE_RIGHT_PARENTHESIS, 3},
        {NODE_IDENTIFIER, 4},
        {NODE_OPERATOR_UNARY, 5},
        {NODE_OPERATOR_NIL_COALESCING, 6},
        {NODE_EOL, 7},
        {NODE_OPERATOR_EQUAL, 8},
        {NODE_OPERATOR_BELOW, 8},
        {NODE_OPERATOR_ABOVE, 8},
        {NODE_OPERATOR_BEQ, 8},
        {NODE_OPERATOR_AEQ, 8},
        {NODE_OPERATOR_NEQ, 8}};

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

    if (parent->numChildren % NODE_CHILDREN_ARRAY_CAPACITY == 0)
    { /* Pokud je počet dětí rovný kapacitě, realokuj paměť */
        parent->children = realloc(parent->children, sizeof(TreeNode *) * (parent->numChildren + NODE_CHILDREN_ARRAY_CAPACITY));
        if (parent->children == NULL)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }
    parent->children[parent->numChildren] = son;
    parent->numChildren++;

    // parent->children = realloc(parent->children, sizeof(TreeNode *) * ++parent->numChildren);
    // if (parent->children == NULL)
    // {
    //     error = ERR_INTERNAL;
    //     return false;
    // }

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
    newNode->token_value.string_value = NULL;

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

    // if (parseTree->label != NULL)
    // {
    //     free(parseTree->label);
    // }

    for (unsigned i = 0; i < parseTree->numChildren; i++)
    {

        dispose(parseTree->children[i]);
    }

    // if(parseTree->token_value.string_value != NULL){
    //     free(parseTree->token_value.string_value);
    // }
    
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

    (*node)->label = NULL;
    (*node)->label = malloc(sizeof(char) * (token.value.string_value->size + 1));
    token.value.string_value->buffer[token.value.string_value->size] = '\0';
    strcpy((*node)->label, token.value.string_value->buffer);
    // if (move_buffer(&(*node)->label, token.value.string_value) != ERR_CODE_OK)
    // {
    //     return false;
    // }

    return true;
}

NodeType topTerminal(Stack *stack, int *topTerminalIndex)
{
    NodeType stackTop = -1;
    for (int i = stack->top; i != -1; i--)
    {
        stackTop = *((NodeType *)stack->frames[i].data);
        if (stackTop != NODE_EXPRESSION &&
            stackTop != NODE_SHIFTER)
        {
            *topTerminalIndex = i;
            return stackTop;
        }
    }

    return -1;
}

bool isRule(Stack *stack, Stack *treeStack)
{
    NodeType stackTop = *((NodeType *)(stack_top(stack)->data));
    //// printf("stackTop: %d\n", *((NodeType *)(stack_top(stack)->data)));
    RuleType *rule = malloc(sizeof(RuleType));
    switch (stackTop)
    {
    case NODE_OPERATOR_UNARY:

        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_EXPRESSION)
        {
            stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
            if (stackTop == NODE_SHIFTER)
            {
                *rule = RULE_UNARY;
                stack_push(treeStack, rule);
                return true;
            }
        }
        break;
    case NODE_IDENTIFIER:

        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_SHIFTER)
        {
            *rule = RULE_ID;
            stack_push(treeStack, rule);
            return true;
        }
        break;
    case NODE_EXPRESSION:
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        switch (stackTop)
        {
        case NODE_OPERATOR_ADD:
            *rule = RULE_ADD;
            break;
        case NODE_OPERATOR_SUB:
            *rule = RULE_SUB;
            break;
        case NODE_OPERATOR_MUL:
            *rule = RULE_MUL;
            break;
        case NODE_OPERATOR_DIV:
            *rule = RULE_DIV;
            break;
        case NODE_OPERATOR_NIL_COALESCING:
            *rule = RULE_COALESCING;
            break;
        case NODE_OPERATOR_EQUAL:
            *rule = RULE_EQUAL;
            break;
        case NODE_OPERATOR_BELOW:
            *rule = RULE_BELOW;
            break;
        case NODE_OPERATOR_ABOVE:
            *rule = RULE_ABOVE;
            break;
        case NODE_OPERATOR_AEQ:
            *rule = RULE_AEQ;
            break;
        case NODE_OPERATOR_BEQ:
            *rule = RULE_BEQ;
            break;
        case NODE_OPERATOR_NEQ:
            *rule = RULE_NEQ;
            break;
        default:
            return false;
        }
        stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
        if (stackTop == NODE_EXPRESSION)
        {
            stackTop = *((NodeType *)(stack->frames[stack->top - 3].data));
            if (stackTop == NODE_SHIFTER)
            {
                stack_push(treeStack, rule);
                return true;
            }
        }

    case NODE_RIGHT_PARENTHESIS:
        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_EXPRESSION)
        {
            stackTop = *((NodeType *)(stack->frames[stack->top - 2].data));
            if (stackTop == NODE_LEFT_PARENTHESIS)
            {
                stackTop = *((NodeType *)(stack->frames[stack->top - 3].data));
                if (stackTop == NODE_SHIFTER)
                {
                    *rule = RULE_PARENTHESES;
                    stack_push(treeStack, rule);
                    return true;
                }
            }
        }
        break;
    default:
        return false;
    }
    free(rule);
    return false;
}

bool pushBehindTerminal(Stack *stack, int topTerminalIndex)
{
    NodeType *tempPtr = malloc(sizeof(NodeType));
    if (tempPtr == NULL)
    {
        return false;
    }

    *tempPtr = NODE_SHIFTER;

    Stack_Frame tempFrame;
    tempFrame.data = tempPtr;

    stack_push(stack, stack_top(stack)->data);

    for (int i = stack->top - 1; i > topTerminalIndex + 1; i--)
    {
        stack->frames[i] = stack->frames[i - 1];
    }

    stack->frames[topTerminalIndex + 1] = tempFrame;
    return true;
}

token_type_t checkForImmediateOperands(token_type_t tokenType, TreeNode *nodeExpression, Stack *stack, Stack *tokenValueStack)
{
    token_t token;
    NodeType *temp = malloc(sizeof(NodeType));
    DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));
    token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));

    switch (tokenType)
    {
    case TOKEN_IDENTIFIER:
        *temp = NODE_IDENTIFIER;
        stack_push(stack, temp);
        break;
    case TOKEN_INT:
        *temp = NODE_INT;
        stack_push(stack, temp);
        tokenType = TOKEN_IDENTIFIER;
        break;
    case TOKEN_DOUBLE:
        *temp = NODE_DOUBLE;
        stack_push(stack, temp);
        tokenType = TOKEN_IDENTIFIER;
        break;
    case TOKEN_NIL:
        *temp = NODE_NIL;
        stack_push(stack, temp);
        tokenType = TOKEN_IDENTIFIER;
        break;
    case TOKEN_DOUBLE_QUOTE:
        token = get_token(file);
        tokenType = TOKEN_IDENTIFIER;
        *temp = NODE_STRING;
        if (token.type != TOKEN_STRING && token.type != TOKEN_DOUBLE_QUOTE)
        {
            return -1;
        }

        if (buffer == NULL)
        {
            return false;
        }
        init_buffer(buffer, BUFFER_INIT_CAPACITY);

        if (token.type == TOKEN_DOUBLE_QUOTE)
        {
            buffer_clear(buffer);
            tokenValuePtr->string_value = buffer;
            stack_push(tokenValueStack, tokenValuePtr);
            stack_push(stack, temp);
            return tokenType;
        }

        if (token.type == TOKEN_STRING)
        {
            move_buffer_to_buffer(buffer, token.value.string_value);
        }

        tokenValuePtr->string_value = buffer;

        token = get_token(file);

        if (token.type != TOKEN_DOUBLE_QUOTE)
        {
            return -1;
        }

        stack_push(tokenValueStack, tokenValuePtr);
        stack_push(stack, temp);
        break;
    case TOKEN_TRIPLE_DOUBLE_QUOTE:
        token = get_token(file);
        tokenType = TOKEN_IDENTIFIER;
        *temp = NODE_STRING;
        if (token.type != TOKEN_STRING && token.type != TOKEN_TRIPLE_DOUBLE_QUOTE)
        {
            return -1;
        }

        if (buffer == NULL)
        {
            return false;
        }
        init_buffer(buffer, BUFFER_INIT_CAPACITY);

        if (token.type == TOKEN_TRIPLE_DOUBLE_QUOTE)
        {
            buffer_clear(buffer);
            tokenValuePtr->string_value = buffer;
            stack_push(tokenValueStack, tokenValuePtr);
            stack_push(stack, temp);
            return tokenType;
        }

        if (token.type == TOKEN_STRING)
        {
            move_buffer_to_buffer(buffer, token.value.string_value);
        }

        tokenValuePtr->string_value = buffer;

        token = get_token(file);

        if (token.type != TOKEN_TRIPLE_DOUBLE_QUOTE)
        {
            return -1;
        }

        stack_push(tokenValueStack, tokenValuePtr);
        stack_push(stack, temp);
        break;
    default:
        break;
    }
    return tokenType;
}

TreeNode *buildTree(Stack *treeStack, TreeNode *nodeExpression, Stack *idTypeStack, Stack *identifier_stack, Stack *tokenValueStack)
{
    RuleType *stackTop = malloc(sizeof(RuleType));
    *stackTop = *((RuleType *)(stack_top(treeStack)->data));
    switch (*stackTop)
    {
    case RULE_ID:;
        NodeType idType = *((NodeType *)(stack_top(idTypeStack)->data));
        TreeNode *id = createNewNode(nodeExpression, idType, true);

        if (idType == NODE_INT || idType == NODE_DOUBLE)
        {
            id->token_value = *((token_value_t *)(stack_top(tokenValueStack)->data));
            stack_pop(tokenValueStack);
        }

        if (idType == NODE_STRING)
        {
            id->label = (*((token_value_t *)(stack_top(tokenValueStack)->data))).string_value->buffer;
            stack_pop(tokenValueStack);
        }

        if (idType == NODE_IDENTIFIER)
        {
            Stack_Frame *frame = stack_top(identifier_stack);
            DynamicBuffer *id_buffer = frame->data;
            id->label = NULL;
            if (move_buffer(&id->label, id_buffer) != ERR_CODE_OK)
            {
                error = ERR_INTERNAL;
                return NULL;
            }
            stack_pop(identifier_stack);
        }

        if (id == NULL)
        {
            return NULL;
        }
        free(stack_top(treeStack)->data);
        free(stack_top(idTypeStack)->data);
        stack_pop(treeStack);
        stack_pop(idTypeStack);
        break;
    case RULE_ADD:
    case RULE_SUB:
    case RULE_MUL:
    case RULE_DIV:
    case RULE_COALESCING:
    case RULE_BELOW:
    case RULE_ABOVE:
    case RULE_BEQ:
    case RULE_AEQ:
    case RULE_EQUAL:
    case RULE_NEQ:;
        TreeNode *expressionLeft = createNewNode(nodeExpression, NODE_EXPRESSION, false);
        if (expressionLeft == NULL)
        {
            return NULL;
        }
        TreeNode *operator= createNewNode(nodeExpression, NODE_OPERATOR_ADD, true);
        if (operator== NULL)
        {
            return NULL;
        }
        switch (*stackTop)
        {
        case RULE_SUB:
            operator->type = NODE_OPERATOR_SUB;
            break;
        case RULE_MUL:
            operator->type = NODE_OPERATOR_MUL;
            break;
        case RULE_DIV:
            operator->type = NODE_OPERATOR_DIV;
            break;
        case RULE_COALESCING:
            operator->type = NODE_OPERATOR_NIL_COALESCING;
            break;
        case RULE_EQUAL:
            operator->type = NODE_OPERATOR_EQUAL;
            break;
        case RULE_BELOW:
            operator->type = NODE_OPERATOR_BELOW;
            break;
        case RULE_ABOVE:
            operator->type = NODE_OPERATOR_ABOVE;
            break;
        case RULE_BEQ:
            operator->type = NODE_OPERATOR_BEQ;
            break;
        case RULE_AEQ:
            operator->type = NODE_OPERATOR_AEQ;
            break;
        case RULE_NEQ:
            operator->type = NODE_OPERATOR_NEQ;
            break;
        default:
            break;
        }
        TreeNode *expressionRight = createNewNode(nodeExpression, NODE_EXPRESSION, false);
        if (expressionLeft == NULL)
        {
            return NULL;
        }
        free(stack_top(treeStack)->data);
        stack_pop(treeStack);
        buildTree(treeStack, expressionRight, idTypeStack, identifier_stack, tokenValueStack);
        buildTree(treeStack, expressionLeft, idTypeStack, identifier_stack, tokenValueStack);
        break;
    case RULE_PARENTHESES:;
        TreeNode *leftParenthesis = createNewNode(nodeExpression, NODE_LEFT_PARENTHESIS, true);
        if (leftParenthesis == NULL)
        {
            return NULL;
        }
        TreeNode *expression = createNewNode(nodeExpression, NODE_EXPRESSION, false);
        if (expression == NULL)
        {
            return NULL;
        }
        TreeNode *rightParenthesis = createNewNode(nodeExpression, NODE_RIGHT_PARENTHESIS, true);
        if (rightParenthesis == NULL)
        {
            return NULL;
        }
        free(stack_top(treeStack)->data);
        stack_pop(treeStack);
        buildTree(treeStack, expression, idTypeStack, identifier_stack, tokenValueStack);
        break;
    case RULE_UNARY:;
        TreeNode *expressionUnary = createNewNode(nodeExpression, NODE_EXPRESSION, false);
        if (expressionUnary == NULL)
        {
            return NULL;
        }
        TreeNode *unary = createNewNode(nodeExpression, NODE_OPERATOR_UNARY, true);
        if (unary == NULL)
        {
            return NULL;
        }
        free(stack_top(treeStack)->data);
        stack_pop(treeStack);
        buildTree(treeStack, expressionUnary, idTypeStack, identifier_stack, tokenValueStack);
        break;
    default:
        return NULL;
    }
    free(stackTop);
    return nodeExpression;
}

bool parseExpression(TreeNode *nodeExpression, token_t prevToken, bool condition)
{
    
    NodeType *endMarkerPtr = malloc(sizeof(NodeType));
    if (endMarkerPtr == NULL)
    {
        return false;
    }
    *endMarkerPtr = NODE_EOL;
    Stack *treeStack = stack_init(STACK_INIT_CAPACITY);
    Stack *stack = stack_init(STACK_INIT_CAPACITY);
    Stack *idTypeStack = stack_init(STACK_INIT_CAPACITY);
    Stack *identifier_labels_stack = stack_init(STACK_INIT_CAPACITY);
    Stack *tokenValueStack = stack_init(STACK_INIT_CAPACITY);
    stack_push(stack, endMarkerPtr);

    token_t token = prevToken;
    token_type_t tokenType;

    NodeType stackTop;
    char tableValue;

    do
    {

        tokenType = token.type;
        //token = get_token(file);
        // printf("TOKEN TYPE: %d\n", token.type);

        /*
        if (tokenType == TOKEN_EOL)
        {
            token = peek_token(file);
            while (token.type == TOKEN_EOL)
            {
                token = get_token(file);
                token = peek_token(file);
            }

            tokenType = checkForImmediateOperands(token.type, nodeExpression, idTypeStack, false);

            if (expressionTokenTypeToNode(tokenType) != -1)
            {
                token = get_token(file);
                tokenType = token.type;
            }
            else
            {
               // printf("Error: tokenType == -1\n");
                tokenType = TOKEN_EOL;
                token.type = TOKEN_EOL;
            }
        }
        */

        if (tokenType == TOKEN_EOL && condition)
        {

            skipEmptyLines(&token);
            tokenType = token.type;
        }

        if (tokenType == TOKEN_EOF)
        {
            tokenType = TOKEN_EOL;
        }

        if (tokenType == TOKEN_LEFT_BRACE && condition)
        {
            tokenType = TOKEN_EOL;
        }

        if (tokenType == TOKEN_IDENTIFIER)
        {
            stack_push(identifier_labels_stack, token.source_value);
        }

        if (tokenType == TOKEN_INT)
        {
            token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));
            tokenValuePtr->int_value = token.value.int_value;
            stack_push(tokenValueStack, tokenValuePtr);
        }

        if (tokenType == TOKEN_DOUBLE)
        {
            token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));
            tokenValuePtr->double_value = token.value.double_value;
            stack_push(tokenValueStack, tokenValuePtr);
        }

        tokenType = checkForImmediateOperands(tokenType, nodeExpression, idTypeStack, tokenValueStack);

        int topTerminalIndex;

        stackTop = topTerminal(stack, &topTerminalIndex);

        if (stackTop == -1)
        {
            //printf("Error: stackTop == -1\n");
            return false;
        }

        if (stackTop == -1)
        {
            //printf("Error: stackTop == -1\n");
            return false;
        }

        NodeType *inputPtr = malloc(sizeof(NodeType));
        if (inputPtr == NULL)
        {
            return false;
        }

        *inputPtr = expressionTokenTypeToNode(tokenType);

        tableValue = nodeTypeToIndex(stackTop, *inputPtr);

        //printf("INPUT: %d\n STACKTOP: %d\n TOKEN TYPE: %d\n TABLE VALUE: %c\n\n", *inputPtr, stackTop, tokenType, tableValue);

        switch (tableValue)
        {
        case '=':
            stack_push(stack, inputPtr);
            token = get_token(file);

            break;
        case '<':;
            if (!pushBehindTerminal(stack, topTerminalIndex))
            {
                return false;
            }

            stack_push(stack, inputPtr);
            token = get_token(file);
            break;
        case '>':
            free(inputPtr);
            if (isRule(stack, treeStack))
            {

                do
                {
                    free(stack_top(stack)->data);
                    stack_pop(stack);
                    stackTop = *((NodeType *)(stack_top(stack)->data));
                } while (stackTop != NODE_SHIFTER);

                free(stack_top(stack)->data);
                stack_pop(stack);

                NodeType *stackTopPtr = malloc(sizeof(NodeType));
                if (stackTopPtr == NULL)
                {
                    return false;
                }

                *stackTopPtr = NODE_EXPRESSION;
                stack_push(stack, stackTopPtr); 
            }
            else
            {
                free(inputPtr);
                // stack_empty(stack);
                // stack_empty(treeStack);
                // stack_empty(idTypeStack);
                stack_free(treeStack);
                stack_free(idTypeStack);
                stack_free(identifier_labels_stack);
                stack_free(tokenValueStack);
                //printf("Error: !isRule\n");
                return false;
            }
            break;
        default:

            free(inputPtr);
            // stack_empty(stack);
            // stack_empty(treeStack);
            // stack_empty(idTypeStack);
            stack_free(treeStack);
            stack_free(idTypeStack);
            stack_free(identifier_labels_stack);
            stack_free(tokenValueStack);
            return false;
        }

    } while (tokenType != TOKEN_EOL || *((NodeType *)(stack_top(stack)->data)) != NODE_EXPRESSION || stack_size(stack) != 2);

    // stack_empty(stack);
    stack_free(stack);

    buildTree(treeStack, nodeExpression, idTypeStack, identifier_labels_stack, tokenValueStack);
    // stack_empty(treeStack);
    // stack_empty(idTypeStack);

    stack_free(treeStack);
    stack_free(idTypeStack);
    stack_free(identifier_labels_stack);
    stack_free(tokenValueStack);

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
    // token_t next = peek_token(file);

    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
        funcParamValue->type = NODE_IDENTIFIER;
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {
            funcParamValue->type = NODE_FUNCTION_CALL;
            funcParamValue->terminal = false;
            if (!parseFuncCall(funcParamValue, prevToken.source_value))
            {
                return false;
            }
            if (!skipEmptyLines(&token))
            {
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

            // token_t next = peek_token(file);

            switch (token.type)
            {
            case TOKEN_IDENTIFIER:
                funcParamRight->type = NODE_IDENTIFIER;
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

            if (token.type == TOKEN_INT || token.type == TOKEN_DOUBLE)
            {
                funcParamValue->token_value = token.value;
            }

            // free_token(next);

            if (!skipEmptyLines(&token))
            {
                return false;
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

        return false;
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

    if (token.type == TOKEN_INT || token.type == TOKEN_DOUBLE)
    {
        funcParamValue->token_value = token.value;
    }

    // free_token(next);
    // if (!skipEmptyLines(&token))
    // {
    //     return false;
    // }
    token_t next_token = get_token(file);

    //printf("token.type: %d\n", next_token.type);
    if (next_token.type == TOKEN_COMMA)
    {
        return parseParameters(funcParams);
    }

    if (next_token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        //printf("right parenthesis\n");
        return true;
    }

    return false;
}

bool parseFuncCall(TreeNode *node, DynamicBuffer *func_name)
{

    node->type = NODE_FUNCTION_CALL;
    TreeNode *funcCallId = createNewNode(node, NODE_IDENTIFIER, true);
    if (funcCallId == NULL)
    {
        return false;
    }

    if (func_name != NULL)
    {
        func_name->buffer[func_name->size] = '\0';
        funcCallId->label = calloc(100,sizeof(char));
        strncpy(funcCallId->label, func_name->buffer, func_name->size + 1);
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

    if (id_name != NULL)
    {
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
    // int numEols;

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    token_t prevToken = token;

    if (token.type == TOKEN_IDENTIFIER)
    {
        token = peek_token(file);
        // while (token.type == TOKEN_EOL)
        // {

        //     token = get_token(file);
        //     token = peek_token(file);
        // }

        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {

            token = get_token(file);
            assignValue->type = NODE_FUNCTION_CALL;

            if (parseFuncCall(assignValue, prevToken.source_value))
            {
                token = get_token(file);
                if (token.type != TOKEN_EOL)
                {
                    return false;
                }
                return true;
            }
            else
            {
                //printf("assign parsed\n");
                return false;
            }
        }
    }

    return parseExpression(assignValue, prevToken, false);
}

bool parseDeclaration(TreeNode *neterminal, bool constant)
{
    symtable_local_data_t *local_data;
    symtable_global_data_t *global_data;
    // create local symtable if there is not currently one
    bool push = false;
    if (local_table == NULL && inBlock)
    {
        local_table = create_local_symtable(ST_LOCAL_INIT_SIZE);

        if (local_table == NULL)
        {
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
    char *key = NULL;
    if (move_buffer(&key, token.source_value) != ERR_CODE_OK)
    {
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
            TreeNode *ident = createNewNode(neterminal, NODE_IDENTIFIER, true);
            if (ident == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }

            ident->label = key;

            if (createNewNode(neterminal, nodeType, true) == NULL)
            {
                return false;
            }

            if (inBlock)
            {
                if (symtable_search(local_table, key, LOCAL_TABLE) != NULL)
                {
                    error = ERR_SEMANTIC_DEFINITION;
                    return false;
                }

                if (symtable_insert(local_table, key, local_data, LOCAL_TABLE) != ERR_CODE_ST_OK)
                {
                    error = ERR_INTERNAL;
                    return false;
                }
            }
            else
            {
                if (symtable_search(global_table, key, GLOBAL_TABLE) != NULL)
                {
                    error = ERR_SEMANTIC_DEFINITION;
                    return false;
                }

                if (symtable_insert(global_table, key, global_data, GLOBAL_TABLE) != ERR_CODE_ST_OK)
                {
                    error = ERR_INTERNAL;
                    return false;
                }
            }

            if (push && inBlock)
            {
                if (stack_push(stack_of_local_tables, local_table) != STACK_SUCCESS)
                {
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

    if (token.type == TOKEN_NIL)
    {
        data_type_t *nil = malloc(sizeof(data_type_t));
        if (nil == NULL)
        {
            error = ERR_INTERNAL;
            return false;
        }
        *nil = DATA_NIL;
        if (inBlock)
            local_data->value = nil;
        else
            global_data->value = nil;
    }

    if (inBlock)
    {
        if (symtable_search(local_table, key, LOCAL_TABLE) != NULL)
        {
            error = ERR_SEMANTIC_DEFINITION;
            return false;
        }

        if (symtable_insert(local_table, key, local_data, LOCAL_TABLE) != ERR_CODE_ST_OK)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }
    else
    {
        if (symtable_search(global_table, key, GLOBAL_TABLE) != NULL)
        {
            error = ERR_SEMANTIC_DEFINITION;
            return false;
        }

        if (symtable_insert(global_table, key, global_data, GLOBAL_TABLE) != ERR_CODE_ST_OK)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }

    if (push && inBlock)
    {
        if (stack_push(stack_of_local_tables, local_table) != STACK_SUCCESS)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }

    token_t prevToken = token;
    if (token.type == TOKEN_IDENTIFIER)
    {

        DynamicBuffer *b = token.source_value;

        token = peek_token(file);

        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {

            free_token(token);
            expression->type = NODE_FUNCTION_CALL;
            token = get_token(file);

            if (parseFuncCall(expression, b))
            {
                free_token(token);
                return true;
            }
            else
            {

                free_token(token);
                return false;
            }
        }
    }

    return parseExpression(expression, prevToken, false);
}

bool parseIfStatement(TreeNode *node, bool isWhile)
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

    if (token.type == TOKEN_KEYWORD_LET && !isWhile)
    {
        ifCond->type = NODE_GUARD_LET;

        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type != TOKEN_IDENTIFIER)
        {
            return false;
        }

        // todo sem guard let variable zmena v tabulce na nilable true a pak pridani do else body_end
        char* key = token.source_value->buffer;
        symtable_record_local_t *local_record = check_stack(stack_of_local_tables, key);
        if(local_record == NULL){
            // we need to check global table
            symtable_record_global_t *global_record = symtable_search(global_table, key, GLOBAL_TABLE);

            if(global_record == NULL){
                // variable not found
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

             if(global_record->data->symbol_type != SYM_CONSTANT){
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            if(!global_record->data->defined){
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

            if(!global_record->data->nilable){
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            global_record->data->nilable = false;

        }else if(local_record != NULL){
            if(local_record->data->symbol_type != SYM_CONSTANT){
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            if(!local_record->data->defined){
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

            if(!local_record->data->nilable){
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }


            local_record->data->nilable = false;
        }else{
            error = ERR_SEMANTIC_NOT_DEFINED;
            return false;
        }




        TreeNode *id = createNewNode(ifCond, NODE_IDENTIFIER, true);
        if (id == NULL)
        {
            return false;
        }

        if (move_buffer(&id->label, token.source_value) != ERR_CODE_OK)
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
    }
    else
    {

        if (!parseExpression(ifCond, token, true))
        {
            return false;
        }
    }

    TreeNode *body = createNewNode(node, NODE_BODY, false);
    if (body == NULL)
    {
        return false;
    }

    inBlock++;
    local_table = NULL;
    if (!parse(body))
    {
        //printf("Error: parse body\n");
        return false;
    }

    inBlock--;

    if (isWhile)
    {
        return true;
    }

    TreeNode *elseStatementBody = createNewNode(node, NODE_BODY, false);
    if (elseStatementBody == NULL)
    {
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_KEYWORD_ELSE)
    {
        if (!skipEmptyLines(&token))
        {
            return false;
        }

        if (token.type != TOKEN_LEFT_BRACE)
        {
            return false;
        }
        inBlock++;
        if (!parse(elseStatementBody))
        {
            return false;
        }
        inBlock--;
        free_token(token);
        token = get_token(file);
        if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
        {
            free_token(token);
            return false;
        }
        free_token(token);
        return true;
    }

    free_token(token);

    return false;
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

error_code_t fill_local_params(local_symtable *local_sym_table, parameter_list_t *param_list)
{
    if (local_sym_table == NULL || param_list == NULL)
    {
        return ERR_INTERNAL;
    }

    first(param_list);
    while (parameter_list_active(param_list))
    {
        function_parameter_t *param = parameter_list_get_active(param_list);
        symtable_local_data_t *data = create_local_data(SYM_VAR, param->data_type, param->nilable, true, NULL);
        if (data == NULL)
        {
            return ERR_INTERNAL;
        }

        if (symtable_insert(local_sym_table, param->name, data, LOCAL_TABLE) != ERR_CODE_ST_OK)
        {
            return ERR_INTERNAL;
        }

        parameter_list_next(param_list);
    }

    return ERR_NONE;
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
    node->type = NODE_DECLARATION_FUNCTION;
    parameter_list_t *param_list = malloc(sizeof(parameter_list_t));
    if (param_list == NULL)
    {
        return false;
    }
    parameter_list_init(param_list);

    // defined - depending if the function has a body
    symtable_global_data_t *data = create_global_data(SYM_FUNC, DATA_NONE, false, false, NULL, param_list);

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

    if (symtable_search(global_table, key, GLOBAL_TABLE) != NULL)
    {
        error = ERR_SEMANTIC_DEFINITION;
        return false;
    }

    int ret = symtable_insert(global_table, key, data, GLOBAL_TABLE);

    if (ret != ERR_CODE_ST_OK)
    {
        error = ERR_INTERNAL;
        return false;
    }

    if (l_brace)
    {

        TreeNode *funcBody = createNewNode(node, NODE_BODY, false);
        bool push = false;
        inBlock++;
        inFunction = true;
        if (local_table == NULL)
        {
            local_table = create_local_symtable(ST_LOCAL_INIT_SIZE);

            if (local_table == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }
            push = true;
        }

        error_code_t e = fill_local_params(local_table, param_list);
        if (e != ERR_NONE)
        {
            error = e;
            return false;
        }

        if (push)
        {
            if (stack_push(stack_of_local_tables, local_table) != STACK_SUCCESS)
            {
                error = ERR_INTERNAL;
                return false;
            }
        }
        
        if (!parse(funcBody))
        {
            return false;
        }

        inBlock--;
        inFunction = false;
    }
    else
    {
        if (!createNewNode(node, NODE_EPSILON, true))
        {
            return false;
        }
    }

    return true;
}

bool parseReturn(TreeNode *node)
{
    token_t token;
    token = get_token(file);

    node->type = NODE_RETURN;

    TreeNode *returnExpression = createNewNode(node, NODE_EXPRESSION, false);
    if (returnExpression == NULL)
    {
        return false;
    }

    token_t prevToken = token;
    if (token.type == TOKEN_IDENTIFIER)
    {

        DynamicBuffer *buff = token.source_value;
        token = peek_token(file);

        if (token.type == TOKEN_LEFT_PARENTHESIS)
        {
            free_token(token);
            token = get_token(file);
            returnExpression->type = NODE_FUNCTION_CALL;

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
    }

    if (!parseExpression(returnExpression, prevToken, false))
    {

        if (prevToken.type != TOKEN_EOL)
        {
            return false;
        }

        TreeNode *epsilon = createNewNode(returnExpression, NODE_EPSILON, false);
        if (epsilon == NULL)
        {
            return false;
        }
    }

    return true;
}

bool parse(TreeNode *startNeterminal)
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

        if (!fill_global_builtin_functions(global_table))
        {
            error = ERR_INTERNAL;
            return false;
        }
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

                nextNeterminal->type = NODE_BODY_END;
                nextNeterminal->terminal = true;

                if (nextNeterminal->label != NULL)
                {
                    // the label now contains key to the local table (or global table if we are in the global scope)
                    // we need to change nilable to false since the variable is now not guarded
                    char *key = nextNeterminal->label;
                    //printf("ending block key: %s\n", key);
                    symtable_record_local_t *record = check_stack(stack_of_local_tables, key);

                    if (record == NULL)
                    {

                        symtable_record_global_t *record_global = symtable_search(global_table, key, GLOBAL_TABLE);
                        if (record_global == NULL)
                        {
                            error = ERR_SEMANTIC_DEFINITION;
                            return false;
                        }

                        record_global->data->nilable = true;
                    }
                    else
                    {
                        record->data->nilable = true;
                    }
                }

                if (!inFunction)
                {
                    stack_pop(stack_of_local_tables); // pop local table (since we are leaving the block) (unless we are in a function, then its popped in parseFuncDeclaration)
                    local_table = NULL;               // set local table to NULL (since we are leaving the block)
                }

                return true;
            }

            return false;

        case TOKEN_IDENTIFIER:;

            DynamicBuffer *buff_copy = token.source_value;
            if (!skipEmptyLines(&token))
            {
                return false;
            }

            switch (token.type)
            {
            case TOKEN_LEFT_PARENTHESIS: // func call

                if (!parseFuncCall(nextNeterminal, buff_copy))
                {
                    return false;
                }

                semantic_result = semantic(nextNeterminal);
                //printf("semantic result func_call: %d\n", semantic_result);
                if (semantic_result != ERR_NONE)
                {
                    error = semantic_result;
                    return false;
                }

                token = get_token(file);

                if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                {
                    return false;
                }

                if (!inBlock)
                    generateFuncCall(nextNeterminal, inBlock);
                break;

            case TOKEN_OPERATOR_ASSIGN: // assign
                nextNeterminal->type = NODE_ASSIGN;
                if (!parseAssign(nextNeterminal, buff_copy))
                {
                    return false;
                }

                semantic_result = semantic(nextNeterminal);
                //printf("semantic result assign: %d\n", semantic_result);
                if (semantic_result != ERR_NONE)
                {
                    error = semantic_result;
                    return false;
                }

                if (!inBlock)
                    generateAssign(nextNeterminal, inBlock);

                break;

            default:
                return false;
            }

            break;
        case TOKEN_KEYWORD_IF:
            nextNeterminal->type = NODE_IF_STATEMENT;

            if (!parseIfStatement(nextNeterminal, false))
            {
                //printf("if statement error\n");
                return false;
            }

            semantic_result = semantic(nextNeterminal);
            //printf("semantic result if: %d\n", semantic_result);
            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            if (!inBlock)
                generateIf(nextNeterminal, inBlock);

            break;
        case TOKEN_KEYWORD_WHILE:
            nextNeterminal->type = NODE_WHILE;

            if (!parseIfStatement(nextNeterminal, true))
            {
                //printf("while error\n");
                return false;
            }

            semantic_result = semantic(nextNeterminal);

            //printf("semantic result while: %d\n", semantic_result);
            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            token = get_token(file);
            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
                return false;
            }

            if (!inBlock)
                generateWhile(nextNeterminal, inBlock);

            break;
        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_VAR:;
            bool constant = token.type == TOKEN_KEYWORD_LET ? true : false;

            nextNeterminal->type = NODE_DECLARATION;
            if (!parseDeclaration(nextNeterminal, constant))
            {

                return false;
            }

            semantic_result = semantic(nextNeterminal);
            //printf("semantic result declaration: %d\n", semantic_result);
            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            if (!inBlock)
                generateDeclaration(nextNeterminal, inBlock);

            free_token(token);

            break;
        case TOKEN_KEYWORD_FUNC:

            if (inBlock || !parseFuncDeclaration(nextNeterminal))
            {
                return false;
            }
            
            nextNeterminal->type = NODE_DECLARATION_FUNCTION;

            semantic_result = semantic(nextNeterminal);

            //printf("semantic result func_decl: %d\n", semantic_result);
            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            
            Stack_Frame *top_frame = stack_top(stack_of_local_tables);
            stack_pop(stack_of_local_tables); // pop local table (since we are leaving the function)
            symtable_free(top_frame->data, LOCAL_TABLE);
            local_table = NULL; // set local table to NULL (since we are leaving the function)
            
            token = get_token(file);

            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
                return false;
            }
            
            generateFuncDeclaration(nextNeterminal, inBlock);
            
            break;
        case TOKEN_KEYWORD_RETURN:
            free_token(token);

            if (!inFunction || !parseReturn(nextNeterminal))
            {
                //printf("return error\n");
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
   // printf("Global table:");
    if (get_size(table) == 0)
    {
       // printf(" is empty\n");
        return;
    }

   // printf("\n");
    for (int i = 0; i < table->capacity; i++)
    {
        symtable_record_global_t *item = table->records[i];
        if (item != NULL)
        {
           // printf("Record: ");
           // printf("data type: %d, symbol type %d, nilable: %d, defined %d, key %s, value %p\n", item->data->data_type, item->data->symbol_type, item->data->nilable, item->data->defined, item->key, item->data->value);
           // printf("Parameters:\n");
            parameter_list_t *list = item->data->parameters;
            if (list == NULL)
                continue;

            if (list->size == SIZE_MAX)
                continue;

            first(list);
            for (int i = 0; i < parameter_list_get_size(list); i++)
            {
                function_parameter_t *param = parameter_list_get_active(list);
               // printf("label: %s, name: %s, data type: %d, nilable: %d\n", param->label, param->name, param->data_type, param->nilable);

                parameter_list_next(list);
            }
           // printf("\n");
        }
    }
}

void print_local_table(local_symtable *table)
{
   // printf("Local table:\n");

    for (int i = 0; i < table->capacity; i++)
    {
        symtable_record_local_t *item = table->records[i];
        if (item != NULL)
        {
           // printf("data type: %d, symbol type %d, nilable: %d, defined %d, key %s, value %p\n", item->data->data_type, item->data->symbol_type, item->data->nilable, item->data->defined, item->key, item->data->value);
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
        "NODE_WHILE",
        "NODE_RETURN",
        "NODE_KEYWORD_LET",
        "NODE_GUARD_LET",
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
        "NODE_UNDERSCORE"};
    return translate[n];
}

void printTree(TreeNode *x, bool *flag, int depth, int isLast)
{
    if (x == NULL)
        return;

    for (int i = 1; i < depth; ++i)
    {
        if (flag[i])
        {
           printf("|   ");
        }
        else
        {
           printf("    ");
        }
    }

    if (depth == 0)
       printf("%s, with value %s\n", node_type_to_string(x->type), x->label);

    else
     if (isLast)
    {
       printf("+--- %s, with value %s\n", node_type_to_string(x->type), x->label);

        flag[depth] = false;
    }
    else
    {
       printf("+--- %s, with value %s\n", node_type_to_string(x->type), x->label);
    }

    for (size_t it = 0; it < x->numChildren; ++it)
    {
        printTree(x->children[it], flag, depth + 1,
                  it == (x->numChildren) - 1);
    }
    flag[depth] = true;
}

void print_stack(Stack *stack)
{
   // printf("Stack:\n");
    for (int i = 0; i < stack_size(stack); i++)
    {
        Stack_Frame *frame = stack_get(stack, i);
        print_local_table(frame->data);
    }
}

int main(void)
{

    stack_of_local_tables = stack_init(STACK_INIT_CAPACITY);
    if (stack_of_local_tables == NULL)
    {
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

    if (parse(startNeterminal))
    {
        error = ERR_NONE;
    }

    //print_global_table(global_table);
    
    //print_stack(stack_of_local_tables);
    bool ar[10] = {true};
    
    //printTree(startNeterminal, ar, 0, 0);
    dispose(startNeterminal);   
    

    symtable_free(global_table, GLOBAL_TABLE);
    stack_free(stack_of_local_tables);

    if (fclose(file) == EOF)
    {
        error = ERR_INTERNAL;
    }

    printf("%d\n", error);
    return error;
}

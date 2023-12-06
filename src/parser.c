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

FILE *file = NULL;
global_symtable *global_table = NULL;
error_code_t error;
static unsigned inBlock = 0;
static bool inFunction = false;
static local_symtable *local_table = NULL;

Stack *stack_of_local_tables = NULL;
Stack *stack_code_gen = NULL;
char *current_function_name = NULL;

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

/**
 * @brief converts token type to node type
 * @param t_type - token type to be converted
 * @return - node type or -1 if conversion is not possible
 */
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

/**
 * @brief converts node type to symtable data type
 * @param n_type - node type to be converted
 * @return - symtable data type or -1 if conversion is not possible
 */
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

/**
 * @brief converts token type of terminal to node type
 * @param inputTokenType - token type to be converted
 * @return - node type or -1 if input type is not terminal
 */
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

/**
 * @brief checks if node type is nilable
 * @param nt - node type to be checked
 * @return - true if node type is nilable, false otherwise
 */
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

/**
 * @brief finds value in precedence table based on terminal nearest to the top of the stack and input
 * @param stackTopNodeType - terminal nearest to the top of the stack
 * @param inputNodeType - input
 * @return - value from precedence table or -1 if input is not valid
 */
char nodeTypeToIndex(NodeType stackTopNodeType, NodeType inputNodeType)
{
    // precedence table
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

/**
 * @brief adds new node to the tree
 * @param parent - node to which the new node will be added
 * @param son - new node
 * @return - true if successful or node is root, false otherwise
 */
bool addNode(TreeNode *parent, TreeNode *son)
{
    if (parent == NULL) // if new node is root
    {
        return true;
    }

    if (son == NULL)
    {
        error = ERR_INTERNAL;
        return false;
    }

    if (parent->numChildren % NODE_CHILDREN_ARRAY_CAPACITY == 0)
    { /* if number of children is same as capacity of array */
        parent->children = realloc(parent->children, sizeof(TreeNode *) * (parent->numChildren + NODE_CHILDREN_ARRAY_CAPACITY));
        if (parent->children == NULL)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }
    parent->children[parent->numChildren] = son;
    parent->numChildren++;

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
    newNode->token_value.string_value = NULL;

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

/**
 * @brief skips comment tokens
 * @param token - token to be checked
 */
void skip_comments(token_t *token)
{
    while (token->type == TOKEN_NONE)
    {
        free_token(*token);
        *token = get_token(file);
    }
}

/**
 * @brief skips EOL tokens
 * @param token - token to be checked
 * @return - number of EOLs
 */
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

    skip_comments(token);

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

/**
 * @brief finds terminal nearest to the top of the stack
 * @param stack - stack to be checked
 * @param topTerminalIndex - variable to store index of the top terminal
 * @return - terminal node type nearest to the top of the stack
 */
NodeType topTerminal(Stack *stack, int *topTerminalIndex)
{
    NodeType stackTop = -1;
    for (int i = stack->top; i != -1; i--)
    {
        stackTop = *((NodeType *)stack->frames[i].data);
        if (stackTop != NODE_EXPRESSION && stackTop != NODE_SHIFTER) // if stackTop is terminal
        {
            *topTerminalIndex = i;

            return stackTop;
        }
    }

    return -1;
}

/**
 * @brief checks if rule can be applied and pushes it to stack
 * @param stack - stack to be checked
 * @param treeStack - stack for storing rules
 * @return - true if rule can be applied, false otherwise
 */
bool isRule(Stack *stack, Stack *treeStack)
{
    NodeType stackTop = *((NodeType *)(stack_top(stack)->data));
    RuleType *rule = malloc(sizeof(RuleType));
    switch (stackTop)
    {
    // expr !
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
    // id
    case NODE_IDENTIFIER:

        stackTop = *((NodeType *)(stack->frames[stack->top - 1].data));
        if (stackTop == NODE_SHIFTER)
        {
            *rule = RULE_ID;
            stack_push(treeStack, rule);
            return true;
        }
        break;
    // expr operator expr
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
    // ( expr )
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

/**
 * @brief pushes shifter behind terminal nearest top of stack
 * @param stack - stack where is shifter pushed
 * @param topTerminalIndex - index of the top terminal in stack
 * @return - true if successful, false otherwise
 */
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

/**
 * @brief checks token type from input and pushes it to stack if it is operand
 * @param tokenType - token type to be checked
 * @param stack - stack where is operand pushed
 * @return TOKEN_IDENTIFIER if there is immediate operand, original token type otherwise
 */
token_type_t checkForImmediateOperands(token_type_t tokenType, Stack *stack)
{
    token_t token;
    NodeType *temp = malloc(sizeof(NodeType));
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
    case TOKEN_STRING:
        *temp = NODE_STRING;
        stack_push(stack, temp);
        tokenType = TOKEN_IDENTIFIER;
        break;
    default:
        break;
    }

    return tokenType;
}

/**
 * @brief builds expression tree from rules stored in stack
 * @param treeStack - stack where are rules stored
 * @param nodeExpression - parent node of created nodes
 * @param idTypeStack - stack where are types of identifiers stored
 * @param identifier_stack - stack where are identifiers stored
 * @param tokenValueStack - stack where are token values of identifiers stored
 * @return - root of expression tree
 */
TreeNode *buildTree(Stack *treeStack, TreeNode *nodeExpression, Stack *idTypeStack, Stack *identifier_stack, Stack *tokenValueStack)
{
    RuleType *stackTop = malloc(sizeof(RuleType));
    *stackTop = *((RuleType *)(stack_top(treeStack)->data));
    switch (*stackTop)
    {

    // id
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

    // expr operator expr
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

    // ( expr )
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

    // expr !
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

/**
 * @brief checks syntax of expression and builds expression tree using precedence analysis
 * @param nodeExpression - parent node of expression tree
 * @param prevToken - saved token before consuming next
 * @param condition - true if expression is condition of while or if, false otherwise
 * @return - true if syntax is correct, false otherwise
 */
bool parseExpression(TreeNode *nodeExpression, token_t prevToken, bool condition)
{
    NodeType *endMarkerPtr = malloc(sizeof(NodeType)); // end marker of expression
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

        if (tokenType == TOKEN_EOL && condition)
        {
            skipEmptyLines(&token);
            tokenType = token.type;
        }

        if (tokenType == TOKEN_EOF) // EOF is also end marker of expression
        {
            tokenType = TOKEN_EOL;
        }

        if (tokenType == TOKEN_LEFT_BRACE && condition) // if expression is condition endmarker == left brace
        {
            tokenType = TOKEN_EOL;
        }

        if (tokenType == TOKEN_IDENTIFIER)
        {
            stack_push(identifier_labels_stack, token.source_value);
        }

        // store constant value to stack
        if (tokenType == TOKEN_INT)
        {
            token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));
            if (tokenValuePtr == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }
            tokenValuePtr->int_value = token.value.int_value;
            stack_push(tokenValueStack, tokenValuePtr);
        }

        if (tokenType == TOKEN_DOUBLE)
        {
            token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));
            if (tokenValuePtr == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }
            tokenValuePtr->double_value = token.value.double_value;
            stack_push(tokenValueStack, tokenValuePtr);
        }

        if (tokenType == TOKEN_STRING)
        {
            DynamicBuffer *buffer = malloc(sizeof(DynamicBuffer));
            if (buffer == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }
            buffer->buffer = token.value.string_value->buffer;
            buffer->size = token.value.string_value->size;
            buffer->capacity = token.value.string_value->capacity;
            token_value_t *tokenValuePtr = malloc(sizeof(token_value_t));
            if (tokenValuePtr == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }
            tokenValuePtr->string_value = buffer;
            stack_push(tokenValueStack, tokenValuePtr);
        }

        tokenType = checkForImmediateOperands(tokenType, idTypeStack);

        int topTerminalIndex;

        stackTop = topTerminal(stack, &topTerminalIndex);

        if (stackTop == -1) // if there is no terminal on stack
        {
            return false;
        }

        NodeType *inputPtr = malloc(sizeof(NodeType));
        if (inputPtr == NULL)
        {
            return false;
        }

        *inputPtr = expressionTokenTypeToNode(tokenType);

        tableValue = nodeTypeToIndex(stackTop, *inputPtr); // get value from precedence table

        switch (tableValue)
        {
        case '=': // push input to stack
            stack_push(stack, inputPtr);
            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }

            break;
        case '<':; // shift
            if (!pushBehindTerminal(stack, topTerminalIndex))
            {
                return false;
            }

            stack_push(stack, inputPtr);
            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }
            break;
        case '>': // reduce
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
            else // exspression is not valid
            {
                free(inputPtr);
                stack_empty(stack);
                stack_empty(treeStack);
                stack_empty(idTypeStack);
                stack_free(treeStack);
                stack_free(idTypeStack);
                stack_free(identifier_labels_stack);
                stack_free(tokenValueStack);
                return false;
            }
            break;
        default: // exspression is not valid

            free(inputPtr);
            stack_empty(stack);
            stack_empty(treeStack);
            stack_empty(idTypeStack);
            stack_empty(tokenValueStack);
            stack_free(treeStack);
            stack_free(idTypeStack);
            stack_free(identifier_labels_stack);
            stack_free(tokenValueStack);
            return false;
        }
    } while (tokenType != TOKEN_EOL || *((NodeType *)(stack_top(stack)->data)) != NODE_EXPRESSION || stack_size(stack) != 2);

    stack_empty(stack);
    stack_free(stack);

    buildTree(treeStack, nodeExpression, idTypeStack, identifier_labels_stack, tokenValueStack);
    stack_empty(treeStack);
    stack_empty(idTypeStack);
    stack_empty(tokenValueStack);

    stack_free(treeStack);
    stack_free(idTypeStack);
    stack_free(identifier_labels_stack);
    stack_free(tokenValueStack);

    return true;
}

/**
 * @brief checks syntax of parameters in function call
 * @param funcParams - parent node of parameters
 * @return - true if syntax is correct, false otherwise
 */
bool parseParameters(TreeNode *funcParams)
{
    // creating new node for parameter
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

    if (token.type == TOKEN_RIGHT_PARENTHESIS) // called function has no parameters
    {

        funcParam->type = NODE_EPSILON;
        funcParam->terminal = true;

        return true;
    }

    // creating new node for parameter value or label
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

        if (token.type == TOKEN_COLON) // if the parameter has label
        {

            if (move_buffer(&funcParam->label, token.source_value) != ERR_CODE_OK)
            {
                return false;
            }

            // creating new node for parameter value
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
            case TOKEN_INT:
                funcParamRight->type = NODE_INT;
                break;
            case TOKEN_DOUBLE:
                funcParamRight->type = NODE_DOUBLE;
                break;
            case TOKEN_NIL:
                funcParamRight->type = NODE_NIL;
                break;
            case TOKEN_STRING:
                funcParamRight->type = NODE_STRING;
                break;
            default:
                return false;
                break;
            }

            // store constant value to node
            if (token.type == TOKEN_INT || token.type == TOKEN_DOUBLE)
            {
                funcParamRight->token_value = token.value;
            }

            if (token.type == TOKEN_STRING)
            {

                move_buffer(&funcParamRight->label, token.value.string_value);
            }

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
    case TOKEN_STRING:
        funcParamValue->type = NODE_STRING;
        break;
    default:
        return false;
    }

    // store constant value to node
    if (token.type == TOKEN_INT || token.type == TOKEN_DOUBLE)
    {
        funcParamValue->token_value = token.value;
    }

    if (token.type == TOKEN_STRING)
    {
        move_buffer(&funcParamValue->label, token.value.string_value);
    }

    token_t next_token = get_token(file);

    skip_comments(&next_token);

    if (next_token.type == TOKEN_UNKNOWN)
    {
        error = ERR_LEX_ANALYSIS;
        return false;
    }

    if (next_token.type == TOKEN_ERROR)
    {
        error = ERR_INTERNAL;
        return false;
    }

    if (next_token.type == TOKEN_COMMA) // if there is another parameter
    {
        return parseParameters(funcParams);
    }

    if (next_token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }

    return false;
}

/**
 * @brief checks syntax of function call
 * @param node - parent node of function call
 * @param func_name - name of called function
 * @return - true if syntax is correct, false otherwise
 */
bool parseFuncCall(TreeNode *node, DynamicBuffer *func_name)
{

    node->type = NODE_FUNCTION_CALL;
    // creating new node for function name
    TreeNode *funcCallId = createNewNode(node, NODE_IDENTIFIER, true);
    if (funcCallId == NULL)
    {
        return false;
    }

    // store function name to node
    if (func_name != NULL)
    {
        func_name->buffer[func_name->size] = '\0';
        funcCallId->label = calloc(100, sizeof(char));
        strncpy(funcCallId->label, func_name->buffer, func_name->size + 1);
    }

    // creating new node for function parameters
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

/**
 * @brief checks syntax of assingment
 * @param assign - parent node of assignment
 * @param id_name - name of identifier on the left side of assignment
 * @return - true if syntax is correct, false otherwise
 */
bool parseAssign(TreeNode *assign, DynamicBuffer *id_name)
{
    // creating new node for identifier on the left side of assignment
    TreeNode *assignId = createNewNode(assign, NODE_IDENTIFIER, true);
    if (assignId == NULL)
    {
        return false;
    }

    // store identifier name to node
    if (id_name != NULL)
    {
        if (move_buffer(&assignId->label, id_name) != ERR_CODE_OK)
        {
            error = ERR_INTERNAL;
            return false;
        }
    }

    // creating new node for value on the right side of assignment
    TreeNode *assignValue = createNewNode(assign, NODE_EXPRESSION, false);
    if (assignValue == NULL)
    {
        return false;
    }

    token_t token;

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    token_t prevToken = token;

    if (token.type == TOKEN_IDENTIFIER)
    {
        token = peek_token(file);

        if (token.type == TOKEN_LEFT_PARENTHESIS) // if right side of assignment is function call
        {

            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }
            assignValue->type = NODE_FUNCTION_CALL;

            if (parseFuncCall(assignValue, prevToken.source_value))
            {
                token = get_token(file);

                skip_comments(&token);

                if (token.type == TOKEN_UNKNOWN)
                {
                    error = ERR_LEX_ANALYSIS;
                    return false;
                }

                if (token.type == TOKEN_ERROR)
                {
                    error = ERR_INTERNAL;
                    return false;
                }

                if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                {
                    return false;
                }

                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return parseExpression(assignValue, prevToken, false); // if right side of assignment is expression
}

/**
 * @brief checks syntax of variable declaration
 * @param neterminal - parent node of declaration
 * @param constant - true if variable is let constant, false otherwise
 * @return - true if syntax is correct, false otherwise
 */
bool parseDeclaration(TreeNode *neterminal, bool constant)
{
    symtable_local_data_t *local_data;
    symtable_global_data_t *global_data;
    // create local symtable if there is not currently one

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

    if (token.type == TOKEN_COLON) // if variable data type is specified
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

        if (dataType == -1) // if data type is not valid
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

        skip_comments(&token);

        if (token.type == TOKEN_UNKNOWN)
        {
            error = ERR_LEX_ANALYSIS;
            return false;
        }

        if (token.type == TOKEN_ERROR)
        {
            error = ERR_INTERNAL;
            return false;
        }

        // if variable is not initialized
        if (token.type != TOKEN_OPERATOR_ASSIGN && (token.type == TOKEN_EOL || (token.type == TOKEN_EOF && !inBlock)))
        {
            // create new node for identifier
            TreeNode *ident = createNewNode(neterminal, NODE_IDENTIFIER, true);
            if (ident == NULL)
            {
                error = ERR_INTERNAL;
                return false;
            }

            ident->label = key;

            // create new node for data type
            if (createNewNode(neterminal, nodeType, true) == NULL)
            {
                return false;
            }

            if (inBlock)
            {
                int index = inFunction ? inBlock : inBlock - 1;
                local_symtable *to_insert = stack_get(stack_of_local_tables, index)->data;
                if (symtable_search(to_insert, key, LOCAL_TABLE) != NULL)
                {
                    error = ERR_SEMANTIC_DEFINITION;
                    return false;
                }

                if (symtable_insert(to_insert, key, local_data, LOCAL_TABLE) != ERR_CODE_ST_OK)
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
    // parent node of declaration is assignment
    TreeNode *declaration = createNewNode(neterminal, NODE_DECLARATION, false);
    if (declaration == NULL)
    {
        return false;
    }

    // creating new node for identifier
    if (createNewNode(declaration, NODE_IDENTIFIER, true) == NULL)
    {
        return false;
    }

    declaration->children[0]->label = key;

    // creating new node for data type
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

    // creating new node for value on the right side of assignment
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
        int index = inFunction ? inBlock : inBlock - 1;
        local_symtable *to_insert = stack_get(stack_of_local_tables, index)->data;
        if (symtable_search(to_insert, key, LOCAL_TABLE) != NULL)
        {
            error = ERR_SEMANTIC_DEFINITION;
            return false;
        }

        if (symtable_insert(to_insert, key, local_data, LOCAL_TABLE) != ERR_CODE_ST_OK)
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

    token_t prevToken = token;
    if (token.type == TOKEN_IDENTIFIER)
    {

        DynamicBuffer *b = token.source_value;

        token = peek_token(file);

        if (token.type == TOKEN_LEFT_PARENTHESIS) // if right side of assignment is function call
        {

            free_token(token);
            expression->type = NODE_FUNCTION_CALL;
            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }

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

    return parseExpression(expression, prevToken, false); // if right side of assignment is expression
}

/**
 * @brief checks syntax of if/while statement
 * @param node - parent node of if/while statement
 * @param isWhile - true if statement is while, false if statement is if
 * @return - true if syntax is correct, false otherwise
 */
bool parseIfStatement(TreeNode *node, bool isWhile)
{
    token_t token;
    // creating new node for condition
    TreeNode *ifCond = createNewNode(node, NODE_EXPRESSION, false);
    if (ifCond == NULL)
    {
        return false;
    }

    if (!skipEmptyLines(&token))
    {
        return false;
    }

    if (token.type == TOKEN_KEYWORD_LET && !isWhile) // if condition is 'let identifier'
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

        char *key = token.source_value->buffer;
        symtable_record_local_t *local_record = check_stack(stack_of_local_tables, key);
        if (local_record == NULL)
        {
            // we need to check global table
            symtable_record_global_t *global_record = symtable_search(global_table, key, GLOBAL_TABLE);

            if (global_record == NULL)
            {
                // variable not found
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

            if (global_record->data->symbol_type != SYM_CONSTANT)
            {
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            if (!global_record->data->defined)
            {
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

            if (!global_record->data->nilable)
            {
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            global_record->data->nilable = false;
        }
        else if (local_record != NULL)
        {
            if (local_record->data->symbol_type != SYM_CONSTANT)
            {
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            if (!local_record->data->defined)
            {
                error = ERR_SEMANTIC_NOT_DEFINED;
                return false;
            }

            if (!local_record->data->nilable)
            {
                error = ERR_SEMANTIC_OTHERS;
                return false;
            }

            local_record->data->nilable = false;
        }
        else
        {
            error = ERR_SEMANTIC_NOT_DEFINED;
            return false;
        }

        // creating new node for identifier in condition
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

    // creating new node for body of if/while statement
    TreeNode *body = createNewNode(node, NODE_BODY, false);
    if (body == NULL)
    {
        return false;
    }

    // checking syntax of if/while body
    inBlock++;
    local_table = NULL;
    if (!parse(body))
    {
        return false;
    }

    inBlock--;

    if (isWhile)
    {
        return true;
    }

    // creating new node for else statement
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
        // checking syntax of else body
        if (!parse(elseStatementBody))
        {
            return false;
        }
        inBlock--;
        free_token(token);
        token = get_token(file);

        skip_comments(&token);

        if (token.type == TOKEN_UNKNOWN)
        {
            error = ERR_LEX_ANALYSIS;
            return false;
        }

        if (token.type == TOKEN_ERROR)
        {
            error = ERR_INTERNAL;
            return false;
        }

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

/**
 * @brief checks syntax of parameter in function declaration
 * @param funcParamList - parent node of parameters
 * @param param_list - list of parameters data for filling symtable
 * @return - true if syntax is correct, false otherwise
 */
bool parseParameter(TreeNode *funcParamList, parameter_list_t *param_list)
{
    function_parameter_t *param = malloc(sizeof(function_parameter_t));

    if (param == NULL)
    {

        return false;
    }

    init_param(param);

    // creating new node for parameter
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

    // creating new node for parameter label
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
        // storing parameter label to symtable
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

    // creating new node for parameter name
    TreeNode *paramName = createNewNode(funcParameter, NODE_IDENTIFIER, true);
    if (paramName == NULL)
    {
        return false;
    }

    // storing parameter name to symtable
    if (move_buffer(&param->name, token.source_value) != ERR_CODE_OK)
    {
        return false;
    }

    paramName->label = param->name;

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

    // creating new node for parameter data type
    TreeNode *paramType = createNewNode(funcParameter, NODE_INT, true);
    if (paramType == NULL)
    {
        return false;
    }

    NodeType n_type = token_type_to_node(token.type);
    if (n_type == -1)
        return false;

    paramType->type = n_type;

    // storing parameter data type to symtable
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

    if (token.type == TOKEN_COMMA) // if there is another parameter
    {
        return parseParameter(funcParamList, param_list);
    }

    if (token.type == TOKEN_RIGHT_PARENTHESIS)
    {
        return true;
    }

    return false;
}

/**
 * @brief fills local symtable with parameters of function declaration
 * @param local_sym_table - local symtable of function
 * @param param_list - list of parameters data for filling symtable
 * @return - error code
 */
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
        symtable_local_data_t *data = create_local_data(SYM_PARAMETER, param->data_type, param->nilable, true, NULL);
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

/**
 * @brief checks syntax of parameters in function declaration
 * @param funcParamList - parent node of parameters
 * @param param_list - list of parameters data for filling symtable
 * @return - true if syntax is correct, false otherwise
 */
bool parseParamList(TreeNode *funcParamList, parameter_list_t *param_list)
{
    token_t token;
    token = peek_token(file);

    if (token.type == TOKEN_RIGHT_PARENTHESIS) // if function has no parameters
    {
        free_token(token);
        token = get_token(file);

        skip_comments(&token);

        if (token.type == TOKEN_UNKNOWN)
        {
            error = ERR_LEX_ANALYSIS;
            return false;
        }

        if (token.type == TOKEN_ERROR)
        {
            error = ERR_INTERNAL;
            return false;
        }

        free_token(token);
        TreeNode *funcParameter = createNewNode(funcParamList, NODE_EPSILON, true);
        if (funcParameter == NULL)
        {
            return false;
        }

        return true;
    }

    return parseParameter(funcParamList, param_list); // checks first parameter
}

/**
 * @brief checks syntax of function declaration
 * @param node - parent node of function declaration
 * @return - true if syntax is correct, false otherwise
 */
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

    // creating new node for function name
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

    // storing function name to symtable

    if (move_buffer(&key, bf) != ERR_CODE_OK)
    {
        return false;
    }

    if (move_buffer(&current_function_name, bf) != ERR_CODE_OK)
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

    // creating new node for function parameters
    TreeNode *funcParamList = createNewNode(node, NODE_PARAM_LIST, false);
    if (funcParamList == NULL)
    {
        return false;
    }

    // checking syntax of function parameters
    if (!parseParamList(funcParamList, param_list))
    {
        return false;
    }

    free_token(token);
    token = get_token(file);

    skip_comments(&token);

    if (token.type == TOKEN_UNKNOWN)
    {
        error = ERR_LEX_ANALYSIS;
        return false;
    }

    if (token.type == TOKEN_ERROR)
    {
        error = ERR_INTERNAL;
        return false;
    }

    // creating new node for function return data type
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

        // storing function return data type to symtable

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
        // need to create one local table for function parameters
        local_symtable *local_table_params = create_local_symtable(ST_LOCAL_INIT_SIZE);
        if (local_table_params == NULL)
        {
            error = ERR_INTERNAL;
            return false;
        }

        fill_local_params(local_table_params, param_list);
        stack_push(stack_of_local_tables, local_table_params);

        // creating new node for function body
        TreeNode *funcBody = createNewNode(node, NODE_BODY, false);
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

            stack_push(stack_of_local_tables, local_table);
        }

        // checking syntax of function body
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

/**
 * @brief checks syntax of return statement
 * @param node - parent node of return statement
 * @return - true if syntax is correct, false otherwise
 */
bool parseReturn(TreeNode *node)
{

    token_t token;
    token = get_token(file);

    skip_comments(&token);

    if (token.type == TOKEN_UNKNOWN)
    {
        error = ERR_LEX_ANALYSIS;
        return false;
    }

    if (token.type == TOKEN_ERROR)
    {
        error = ERR_INTERNAL;
        return false;
    }

    node->type = NODE_RETURN;

    // creating new node for return value
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

        if (token.type == TOKEN_LEFT_PARENTHESIS) // if return value is function call
        {
            free_token(token);
            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }

            returnExpression->type = NODE_FUNCTION_CALL;

            if (!parseFuncCall(returnExpression, buff))
            {
                return false;
            }
            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }

            if (token.type == TOKEN_EOL)
            {
                free_token(token);
                return true;
            }
            free_token(token);
            return false;
        }
    }

    // if return value is expression
    if (!parseExpression(returnExpression, prevToken, false))
    {

        if (prevToken.type != TOKEN_EOL)
        {
            return false;
        }

        // if return does not have value
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

    int index = inFunction ? inBlock : inBlock - 1;                                                        // since if we are in function, there is one more block on the stack (block with parameters)
    if ((local_table == NULL || inFunction) && inBlock && stack_get(stack_of_local_tables, index) == NULL) // function solves this
    {
        local_table = create_local_symtable(ST_LOCAL_INIT_SIZE);

        if (local_table == NULL)
        {
            error = ERR_INTERNAL;
            return false;
        }

        stack_push(stack_of_local_tables, local_table);
    }

    token_t token;
    if (!skipEmptyLines(&token))
    {
        return false;
    }

    // consume tokens until EOF
    while (token.type != TOKEN_EOF)
    {
        // creating new node for next command
        TreeNode *nextNeterminal = createNewNode(startNeterminal, NODE_IDENTIFIER, false);
        if (nextNeterminal == NULL)
        {
            return false;
        }

        switch (token.type)
        {
        // }
        case TOKEN_RIGHT_BRACE:
            // end of body
            if (inBlock)
            {

                nextNeterminal->type = NODE_BODY_END;
                nextNeterminal->terminal = true;

                if (nextNeterminal->label != NULL)
                {
                    // the label now contains key to the local table (or global table if we are in the global scope)
                    // we need to change nilable to false since the variable is now not guarded
                    char *key = nextNeterminal->label;
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

                if (inBlock != 1 || !inFunction)
                {
                    stack_pop(stack_of_local_tables); // pop local table (since we are leaving the block) (unless we are in a function, then its popped in parseFuncDeclaration)
                    local_table = NULL;               // set local table to NULL (since we are leaving the block)
                }

                return true;
            }

            // } in main body
            return false;

        // func call or assignment
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
                // semantic check of function call
                semantic_result = semantic(nextNeterminal);

                if (semantic_result != ERR_NONE)
                {
                    error = semantic_result;
                    return false;
                }

                token = get_token(file);

                skip_comments(&token);

                if (token.type == TOKEN_UNKNOWN)
                {
                    error = ERR_LEX_ANALYSIS;
                    return false;
                }

                if (token.type == TOKEN_ERROR)
                {
                    error = ERR_INTERNAL;
                    return false;
                }

                if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
                {
                    return false;
                }

                // code generation of function call
                if (!inBlock)
                {
                    generateFuncCall(nextNeterminal);
                }

                break;

            case TOKEN_OPERATOR_ASSIGN: // assignment
                nextNeterminal->type = NODE_ASSIGN;
                if (!parseAssign(nextNeterminal, buff_copy))
                {
                    return false;
                }

                // semantic check of assignment
                semantic_result = semantic(nextNeterminal);

                if (semantic_result != ERR_NONE)
                {
                    error = semantic_result;
                    return false;
                }

                // code generation of assignment
                if (!inBlock)
                {
                    generateAssign(nextNeterminal);
                }

                break;

            default:
                return false;
            }

            break;

        // if else statement
        case TOKEN_KEYWORD_IF:
            nextNeterminal->type = NODE_IF_STATEMENT;

            if (!parseIfStatement(nextNeterminal, false))
            {
                return false;
            }

            // semantic check of if else statement
            semantic_result = semantic(nextNeterminal);

            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            // code generation of if else statement
            if (!inBlock)
            {
                generateIf(nextNeterminal);
            }

            break;

        // while statement
        case TOKEN_KEYWORD_WHILE:
            nextNeterminal->type = NODE_WHILE;

            if (!parseIfStatement(nextNeterminal, true))
            {
                return false;
            }

            // semantic check of while statement
            semantic_result = semantic(nextNeterminal);

            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }

            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
                return false;
            }

            // code generation of while statement
            if (!inBlock)
            {
                generateWhile(nextNeterminal);
            }

            break;

        // declaration
        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_VAR:;
            bool constant = token.type == TOKEN_KEYWORD_LET ? true : false;

            nextNeterminal->type = NODE_DECLARATION;
            if (!parseDeclaration(nextNeterminal, constant))
            {

                return false;
            }

            // semantic check of declaration
            semantic_result = semantic(nextNeterminal);

            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            // code generation of declaration
            if (!inBlock)
            {
                generateDeclaration(nextNeterminal);
            }

            free_token(token);

            break;

        // function declaration
        case TOKEN_KEYWORD_FUNC:

            current_function_name = NULL;

            if (inBlock || !parseFuncDeclaration(nextNeterminal))
            {
                return false;
            }

            nextNeterminal->type = NODE_DECLARATION_FUNCTION;

            // semantic check of function declaration
            semantic_result = semantic(nextNeterminal);

            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            Stack_Frame *top_frame = stack_top(stack_of_local_tables);
            stack_pop(stack_of_local_tables); // pop local table (since we are leaving the function)
            stack_pop(stack_of_local_tables); // pop one more (since parameters are in a separate block)
            symtable_free(top_frame->data, LOCAL_TABLE);
            local_table = NULL; // set local table to NULL (since we are leaving the function)

            token = get_token(file);

            skip_comments(&token);

            if (token.type == TOKEN_UNKNOWN)
            {
                error = ERR_LEX_ANALYSIS;
                return false;
            }

            if (token.type == TOKEN_ERROR)
            {
                error = ERR_INTERNAL;
                return false;
            }


            if (token.type != TOKEN_EOL && token.type != TOKEN_EOF)
            {
                return false;
            }

            // code generation of function declaration
            generateFuncDeclaration(nextNeterminal);

            break;

        // return
        case TOKEN_KEYWORD_RETURN:
            free_token(token);

            if (!inFunction || !parseReturn(nextNeterminal))
            {
                return false;
            }

            // semantic check of return
            semantic_result = semantic(nextNeterminal);
            if (semantic_result != ERR_NONE)
            {
                error = semantic_result;
                return false;
            }

            break;

        case TOKEN_UNKNOWN: // lex error
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

    if (inBlock) // if we are in block, we need to close it
    {
        return false;
    }

    return true;
}

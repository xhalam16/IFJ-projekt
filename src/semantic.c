/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: semantic.c
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */



#include "header_files/semantic.h"


bool in_body_neterminal = false;
bool set_by_variable = false;
bool next_identifier_unwrapped = false;
bool scan_for_coal = false;
bool coal_found = false;
static bool expression_nilable_bool = false;
static bool same_symbol_assign = false;
static bool sem_ret = false;
static bool expression_immediate = true;
static data_type_t previous = DATA_NONE;
static data_type_t before_nil = DATA_NONE;
static bool divison = false;


/**
 * @brief Checks if node is neterminal
 * @param node Node to check
 * @return True if node is neterminal, false otherwise
*/
bool is_neterminal(TreeNode *node){
    return !node->terminal;
}


/**
 * @brief Recursively checks how many return statements are in the subtree
 * @param node Root of the subtree
 * @param reset If true, the static variable count is reset to 0 (should be called with true, when the function is called for the first time)
 * @return Number of return statements in the subtree
*/
int return_count(TreeNode *node, bool reset){
    static int count = 0;

    if(reset){
        count = 0;
    }

    for(int i = 0; i < node->numChildren; i++){
        if(node->children[i]->type == NODE_RETURN){
            count++;
        }
        return_count(node->children[i], false);
    }
    return count;
}

/**
 * @brief Gets all children of node except NODE_EPSILON
 * @param node Node to get children from
 * @return Number of children
*/
size_t get_num_children(TreeNode *node){
    size_t count = 0;
    for(int i = 0; i < node->numChildren; i++){
        if(node->children[i] != NULL && node->children[i]->type != NODE_EPSILON){
            count++;
        }
    }
    return count;
}

/**
 * @brief Checks whether the node is binary arithmetic operator
 * @param type Type of the node
 * @return True if the node type is binary arithmetic operator, false otherwise
*/
bool is_binary_arithmetic(NodeType type){
    return type == NODE_OPERATOR_ADD || type == NODE_OPERATOR_SUB || type == NODE_OPERATOR_MUL || type == NODE_OPERATOR_DIV || type == NODE_OPERATOR_NIL_COALESCING;
}

/**
 * @brief Checks whether the node is binary relation operator
 * @param type Type of the node
 * @return True if the node type is binary relation operator, false otherwise
*/
bool is_binary_relation(NodeType type){
   switch (type)
   {
    case NODE_OPERATOR_ABOVE:
    case NODE_OPERATOR_AEQ:
    case NODE_OPERATOR_BELOW:
    case NODE_OPERATOR_BEQ:
    case NODE_OPERATOR_EQUAL:
    case NODE_OPERATOR_NEQ:
        return true;
    default:
        return false;
   }
}


/**
 * @brief Checks whether the node is immediate operand
 * @param type Type of the node
 * @return True if the node type is an immediate operand, false otherwise
*/
bool is_immediate_operand(NodeType type){
    return type == NODE_INT || type == NODE_DOUBLE || type == NODE_STRING || type == NODE_NIL;
}


/**
 * @brief Checks the stack of local tables for some identifier
 * @param local_tables Stack of local tables
 * @param identifier Identifier to search for
 * @returns Pointer to the record if found, NULL otherwise
 * @warning This function expects only local tables on the stack, could lead to undefined behaviour if there are other types of data on the stack
 * @note We check from top to bottom
*/
symtable_record_local_t* check_stack(Stack* local_tables, char* identifier){
    // we need to check from top to bottom
    for(int i = local_tables->top; i >= 0; i--){
        Stack_Frame* frame = stack_get(local_tables, i);
        local_symtable* table = (local_symtable*)frame->data;
        
        symtable_record_local_t *record = symtable_search(table, identifier, LOCAL_TABLE);
        
        if(record != NULL){
            // we found the identifier in the local table
            return record;
        }
    }
    return NULL;
    
}

/**
 * @brief Gets the nth record of some identifier in the stack of local tables (n == 0 equals to the first record)
 * @param local_tables Stack of local tables
 * @param identifier Identifier to search for
 * @param n Number of the record to get
 * @returns Pointer to the record if found, NULL otherwise
 * @warning This function expects only local tables on the stack, could lead to undefined behaviour if there are other types of data on the stack
 * @note n == 0 equals to the first record
*/

symtable_record_local_t* get_nth_record(Stack* local_tables, char* identifier, int n){
    // we need to check from top to bottom
    int count = 0;
    for(int i = local_tables->top; i >= 0; i--){
        Stack_Frame* frame = stack_get(local_tables, i);
        local_symtable* table = (local_symtable*)frame->data;

        symtable_record_local_t *record = symtable_search(table, identifier, LOCAL_TABLE);

        if(record != NULL){
            // we found the identifier in the local table
            if(count == n){
                return record;
            }
            count++;
        }
    }

 
    return NULL;
    
}

/**
 * @brief Checks if the data types are compatible in an expression, defined by the language specification
 * @param type1 First data type
 * @param type2 Second data type
 * @param coal_found True if the coallescing operator was found, false otherwise
 * @returns True if the data types are compatible, false otherwise
 * @note This function is used within expression semantic analysis
*/
bool is_datatype_compatible(data_type_t type1, data_type_t type2, bool coal_found, bool division){

    if(division){
        return( type1 == DATA_INT && type2 == DATA_INT) || (type1 == DATA_DOUBLE && type2 == DATA_DOUBLE);
    }


    if(type2 == DATA_NIL && coal_found){
        // nil cannot be on the right side of coallescing operator
        return false;
    }

    if(type1 == DATA_NIL){
        // nil is compatible only if the coallescing operator was found
        return coal_found;
    }


    if(type1 == type2){
        return true;
    }

    if(type1 == DATA_INT && type2 == DATA_DOUBLE){
        return true;
    }

    if(type1 == DATA_DOUBLE && type2 == DATA_INT){
        return true;
    }

    return false;
}

/**
 * @brief Checks if the data types are compatible in an assignment, defined by the language specification
 * @param l_type Left side data type
 * @param r_type Right side data type
 * @param r_value_immediate True if the right side is an immediate value, false otherwise
 * @param coal_found True if the coallescing operator was found, false otherwise
 * @param l_type_nilable True if the left side is nilable, false otherwise
 * @param r_type_nilable True if the right side is nilable, false otherwise
 * @returns True if the data types are compatible, false otherwise
 * @note This function is used within assignment semantic analysis (declaration and assignment)
*/
bool is_assign_compatible(data_type_t l_type, data_type_t r_type, bool r_value_immediate, bool coal_found, bool l_type_nilable, bool r_type_nilable){

    if(l_type == r_type){
        return true;
    }

    if(l_type == DATA_DOUBLE && r_type == DATA_INT && r_value_immediate){
        return true;
    }

    if(!l_type_nilable && r_type_nilable && coal_found){
        return true;
    }

    if(l_type_nilable && r_type == DATA_NIL){
        return true;
    }


    return false;
}


/**
 * @brief Checks if the data types are compatible in a relation expression, defined by the language specification
 * @param type1 First data type
 * @param type2 Second data type
 * @param type1_immediate True if the first data type is an immediate value, false otherwise
 * @param type2_immediate True if the second data type is an immediate value, false otherwise
 * @returns True if the data types are compatible, false otherwise
*/
bool types_compatible_relation(data_type_t type1, data_type_t type2, bool type1_immediate, bool type2_immediate){
    if (type1_immediate && type2_immediate) {
    // if both sides are immediate, double and int are compatible
        return (type1 == DATA_INT && type2 == DATA_DOUBLE) || (type1 == DATA_DOUBLE && type2 == DATA_INT) || (type1 == type2);
    } else if (type1_immediate || type2_immediate) {
    // Either type1 or type2 is immediate

        return (type1_immediate && type1 == DATA_INT && (type2 == DATA_INT || type2 == DATA_DOUBLE)) ||
            (type2_immediate && type2 == DATA_INT && (type1 == DATA_INT || type1 == DATA_DOUBLE)) ||
            type1 == type2;
    } else {
        // Neither type1 nor type2 is immediate
        return type1 == type2;
    }
}


/**
 * @brief Checks if the node is NOT an artihmetic operator
 * @param node Node to check
 * @returns True if the node is NOT an arithmetic operator, false otherwise
*/
bool is_not_arithmetic(TreeNode* node){
    NodeType except[] = {
        NODE_OPERATOR_ABOVE,
        NODE_OPERATOR_AEQ,
        NODE_OPERATOR_BELOW,
        NODE_OPERATOR_BEQ,
        NODE_OPERATOR_EQUAL,
        NODE_OPERATOR_NEQ,
    };

    for(int i = 0; i < node->numChildren; i++){
        for(int j = 0; j < sizeof(except) / sizeof(NodeType); j++){
            if(node->children[i]->type == except[j]){
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Runs a semantic analysis on a artihmetic expression, uses recursion
 * @param node Root of the expression
 * @param data_type Pointer to the data type of the expression (it changes during the analysis)
 * @param local_tables Stack of local tables
 * @param reset If true, the static variables are reset to their default values (should be called with true, when the function is called for the first time)
 * @returns Error code
 * @warning This function works only on arithmetic expressions, it does not work on relation expressions
 * @note Relation expressions are handled by semantic_relation_expression
*/
error_code_t semantic_arithmetic_expression(TreeNode* node, data_type_t *data_type, Stack* local_tables, bool reset){
    // data_type will be set to the type of the expression
    // this function checks if the arithmetic expression is valid

    // NODE_EXPRESSION is node
    // child 0 - expression (operand)
    // child 1 - operator (binary or unary) (only unary - NODE_OPERATOR_UNARY (exclamation mark) is supported)
    // child 2 - expression (operand)
    // ... (more operands and operators)

    // we need to check if the operands are valid
    // for example, we cannot add string to int
    // int (+-*) int = int, double (+-*) double = double
    // double (+-*) int_LITERAL = double (int_LITERAL implicitly converted to double)
    // x ... Int, Double literal (+-*/) x = error, other way around is ok
    // / - division, for int - celociselne deleni, for double - normal division
    static bool first_run = true;

   

    if(reset){
        // since this function is recursive, we need to reset the static and global variables
        first_run = true;
        previous = DATA_NONE;
        before_nil = DATA_NONE;
        expression_immediate = true;
        divison = false;
        expression_nilable_bool = false;
    }
        

    if(first_run){
        /* to reduce the amount of code, this branch is executed only once and handles 
         expressions that can be solved instantly */

        if(is_not_arithmetic(node)){
            // determine the type of error
            return sem_ret ? ERR_SEMANTIC_FUNC : ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

        if(node->numChildren == 1 && node->children[0]->type == NODE_NIL){
            // if the expression contains only one operand and it is nil immediate
            *data_type = DATA_NIL;
            return ERR_NONE;
        }

        if(node->numChildren == 1){
            // if the expression contains only one operand
            // its good for checking immediate nil and variables with value nil
            TreeNode* child = node->children[0];
            if(child->type == NODE_NIL){
                *data_type = DATA_NIL;
                return ERR_NONE;
            }
        }
        if(node->numChildren >= 2 && node->children[1]->type == NODE_OPERATOR_UNARY){
            // we detected unary operator
            next_identifier_unwrapped = true;
        }
            
        first_run = false;
    }

    

    // iterate through all children
    for(int i = 0; i < node->numChildren; i++){
        TreeNode* child = node->children[i];
        
        if(child->type == NODE_EXPRESSION){
            // if we found another expression, lets check if there is unary operator and set flag
            if(child->numChildren >= 2 && child->children[1]->type == NODE_OPERATOR_UNARY){
                next_identifier_unwrapped = true;
            }
            
            // now lets call the function recursively and return error code if there is one
            error_code_t er = semantic_arithmetic_expression(child, data_type, local_tables, false);
            if(er != ERR_NONE){
                return er;
            }
        }
        else{ // its not expression, so lets check if its immediate operand, identifier or binary operator
            if(is_immediate_operand(child->type)){
                
                // if its operand, we need to check if the data type is compatible with the previous operand
                // if its first operand, this if will be skipped
                if(*data_type != DATA_NONE){
                    data_type_t child_type = node_type_to_data(child->type);
                    if(!is_datatype_compatible(*data_type, child_type, coal_found, divison)){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                    if((*data_type == DATA_DOUBLE && child_type == DATA_INT)){
                       continue;
                    }else if(*data_type == DATA_INT && child_type == DATA_DOUBLE && set_by_variable){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }
                }

                if(next_identifier_unwrapped){
                    // we cannot use unwrapping operator on any immediate value
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }

                // set flag that it is immediate value
                set_by_variable = false;
                // change the data type of the expression
                *data_type = node_type_to_data(child->type);
                previous = *data_type;

                return ERR_NONE;

            }else if(child->type == NODE_IDENTIFIER){
                // its an identifier, we need to find information about it in and check the expression
                expression_immediate = false;
                

                // find the record, or return error if the identifier is not defined
                symtable_record_local_t* record = check_stack(local_tables, child->label);
                if(same_symbol_assign){
    
                    record = get_nth_record(local_tables, child->label, 1);
                    same_symbol_assign = false;
                }

                data_type_t var_data_type = DATA_NONE;
                bool var_nilable = false;
                data_type_t *is_nil = NULL;
                if(record == NULL){
                    // we did not find it in local tables, so we need to check global table
                    symtable_record_global_t* glob_record = symtable_search(global_table, child->label, GLOBAL_TABLE);

                    if(glob_record == NULL){
                        // we did not find it in global table, so we return error
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                    

                    if(!glob_record->data->defined){
                        // its not defined, so we return error
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }


                    var_data_type = glob_record->data->data_type;
                    var_nilable = glob_record->data->nilable;
                    if(coal_found) var_nilable = false;

                    is_nil = glob_record->data->value;

                    

                }else{
                    // we found the record in local tables
                    if(!record->data->defined){
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }


                    var_data_type = record->data->data_type;
                    var_nilable = record->data->nilable;
                    if(coal_found) var_nilable = false;
                    is_nil = record->data->value;
                }


                if(is_nil != NULL && *is_nil == DATA_NIL){
                    // variable has value nil
                    if(!var_nilable){
                        return ERR_SEMANTIC_OTHERS;
                    }

                    if(!next_identifier_unwrapped && !coal_found){
                        before_nil = var_data_type;
                        var_data_type = DATA_NIL;        
                    }
                    
                }

                // if we use unwrapping operator on something that is not nilable, we return error
                if(next_identifier_unwrapped && !var_nilable){ 
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }

                if(var_nilable){
                    // if the variable is nilable, we need to check for unwrapping operator or coallescing operator
                    if(!next_identifier_unwrapped){
                        expression_nilable_bool = true;
                        scan_for_coal = true;
                    }else{
                        scan_for_coal = false;
                        expression_nilable_bool = false;
                        next_identifier_unwrapped = false;
                    }

                }
                

                if(*data_type != DATA_NONE){
                    if(!is_datatype_compatible(*data_type, var_data_type, coal_found, divison)){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                    if((*data_type == DATA_DOUBLE && var_data_type == DATA_INT)){
                        // this covers either 3.2 - x, where x is Int
                       return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                    if(coal_found){
                        if(var_data_type != previous){
                            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                        }
                    }else{
                        if(*data_type != var_data_type && set_by_variable){
                            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                        }
                    }
                    


                }
                previous = before_nil == DATA_NONE ? var_data_type : before_nil;
                *data_type = var_data_type;
                set_by_variable = true;

                return ERR_NONE;

            }else if(is_binary_arithmetic(child->type)){

                divison = child->type == NODE_OPERATOR_DIV;

                // if we are scanning for coallescing operator, we need to check if the operator is coallescing
                // else stop scanning
                if(scan_for_coal && child->type != NODE_OPERATOR_NIL_COALESCING){
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }else{
                    scan_for_coal = false;

                }


                // this is not compatible, since we can only add strings
                if(*data_type == DATA_STRING && child->type != NODE_OPERATOR_ADD && child->type != NODE_OPERATOR_NIL_COALESCING){
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }
                

                // this is also not compatible
                if(*data_type == DATA_NIL && child->type != NODE_OPERATOR_NIL_COALESCING){
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }

                coal_found = child->type == NODE_OPERATOR_NIL_COALESCING;

                if(coal_found){
                    expression_nilable_bool = false;
                }

            }

        }

        

    }

    
    // if we are still scanning for coallescing operator, we return error
    if(scan_for_coal){
        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
    }

    return ERR_NONE;
}

/**
 * @brief Checks if the expression is nilable or contains nil literal
 * @param expression Expression to check
 * @returns True if the expression is nilable, false otherwise
*/
bool expression_nilable(TreeNode* expression){

    if(expression->type == NODE_NIL){
        return true;
    }

    if(expression->type == NODE_IDENTIFIER){
        symtable_record_local_t* record = check_stack(stack_of_local_tables, expression->label);
        if(record == NULL){
            symtable_record_global_t* glob_record = symtable_search(global_table, expression->label, GLOBAL_TABLE);
            if(glob_record == NULL){
                return false;
            }

            return glob_record->data->nilable;
        }else{
            return record->data->nilable;
        }
    }

    if(expression->type == NODE_EXPRESSION){
        if(expression->numChildren == 1){
            return expression_nilable(expression->children[0]);
        }
    }

    return false;
}


/**
 * @brief Runs a semantic analysis on a relation expression, uses recursion
 * @param node Root of the expression
 * @param local_tables Stack of local tables
 * @returns Error code
 * @warning This function works only on relation expressions, it does not work on arithmetic expressions
*/
error_code_t semantic_relation_expression(TreeNode* node, Stack* local_tables){
    TreeNode* expression = node;

    // if the expression is wrapped in parentheses, we need to get the expression
    if(node->children[0]->type == NODE_LEFT_PARENTHESIS){
        expression = node->children[1];
    }
    
    // get left and right expression
    TreeNode* l_expression = expression->children[0];
    TreeNode* r_expression = expression->children[2];


    // get the operator
    TreeNode* operator = expression->children[1];

    if(l_expression == NULL || r_expression == NULL || operator == NULL){
        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
    }

    

    data_type_t l_type = DATA_NONE;
    data_type_t r_type = DATA_NONE;

    // first we need to validate the expressions    
    error_code_t er = semantic_arithmetic_expression(l_expression, &l_type, local_tables, true);
    if(er != ERR_NONE){
        return er;
    }


    bool l_immediate = !set_by_variable;

    
    er = semantic_arithmetic_expression(r_expression, &r_type, local_tables, true);
    
    if(er != ERR_NONE){
        return er;
    }

    bool r_immediate = !set_by_variable;

    bool r_expression_nilable = expression_nilable(r_expression);
    bool l_expression_nilable = expression_nilable(l_expression);


    // relation operators except == and != cannot have nilable operands
    // 

    

    switch(operator->type){
        case NODE_OPERATOR_ABOVE:
        case NODE_OPERATOR_AEQ:
        case NODE_OPERATOR_BELOW:
        case NODE_OPERATOR_BEQ:
            // for these operators, neither side can be nilable
            if(r_expression_nilable || l_expression_nilable){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            // check if the types are compatible
            if(!types_compatible_relation(l_type, r_type, l_immediate, r_immediate)){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            break;
        

        case NODE_OPERATOR_EQUAL:
        case NODE_OPERATOR_NEQ:
            // for these operators, both sides can be nilable
            // check if the types are compatible
            
            if(!types_compatible_relation(l_type, r_type, l_immediate, r_immediate)){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            break;
        default:
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
    }

    return ERR_NONE;
}


/**
 * @brief Checks if a node is considered valid term
 * @param node Node to check
 * @param stack_of_local_tables Stack of local tables
 * @returns Error code
 * @note This function is used during write built-in function call semantic analysis
*/
error_code_t is_term(TreeNode* node, Stack* stack_of_local_tables){
    switch (node->type)
    {
    case NODE_INT:
    case NODE_DOUBLE:
    case NODE_STRING:
    case NODE_NIL:
        // all of these are considered valid terms
        return ERR_NONE;
    

    case NODE_IDENTIFIER:;
        // identifier is valid term if it is defined
        char* label = node->label;
        symtable_record_local_t* record = check_stack(stack_of_local_tables, label);

        if(record == NULL){
            symtable_record_global_t* glob_record = symtable_search(global_table, label, GLOBAL_TABLE);
            if(glob_record == NULL){
                return ERR_SEMANTIC_NOT_DEFINED;
            }

            return glob_record->data->defined ? ERR_NONE : ERR_SEMANTIC_NOT_DEFINED;
        }else{
            return record->data->defined ? ERR_NONE : ERR_SEMANTIC_NOT_DEFINED;
        }

    default:
        return ERR_SEMANTIC_FUNC;
        break;
    }
}


/**
 * @brief Helper function, checks if an identifier is in a (sub)tree
 * @param subtree Root of the subtree
 * @param identifier Identifier to search for
 * @returns True if the identifier is in the subtree, false otherwise
*/
bool identifier_in_subtree(TreeNode* subtree, const char* identifier) {
    // Check if the current node is an identifier with matching label
    if (subtree->type == NODE_IDENTIFIER && strcmp(subtree->label, identifier) == 0) {
        return true;
    }

    // Check corresponding children recursively
    for (int i = 0; i < subtree->numChildren; i++) {
        if (identifier_in_subtree(subtree->children[i], identifier)) {
            return true;
        }
    }

    return false;  // If not found in the current node or its children, return false
}


/**
 * @brief Checks if a same identifier is in both subtrees
 * @param subtree1 Root of the first subtree
 * @param subtree2 Root of the second subtree
 * @param identifier Identifier to search for
 * @returns True if the identifier is in both subtrees, false otherwise
*/
bool same_identifier_in_subtrees(TreeNode* subtree1, TreeNode* subtree2, const char* identifier) {
    return identifier_in_subtree(subtree1, identifier) && identifier_in_subtree(subtree2, identifier);
}
  


/**
 * @brief Runs a semantic analysis on a function call
 * @param node Root of the function call
 * @param local_tables Stack of local tables
 * @returns Error code
*/
error_code_t semantic_func_call(TreeNode* node, Stack* local_tables){
    TreeNode *function_name = node->children[0];

    //extract parameters and function name from tree
    TreeNode *param_list = node->children[1];
    char* f_name = function_name->label;


    // check if it exists, and if it's defined and also if it's a function
    symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
    if(record == NULL){
        return ERR_SEMANTIC_DEFINITION;
    }

    if(record->data->symbol_type != SYM_FUNC){
        return ERR_SEMANTIC_DEFINITION;
    }

    // extract parameters from the function definition
    parameter_list_t *param_list_table = record->data->parameters;
    size_t param_count = parameter_list_get_size(param_list_table);

    if(param_count == SIZE_MAX){
        // the function has infinite parameters, so we need to check if the passed parameters are terms or no parameters
        // if they are not, we return error

        for(int i = 0; i < param_list->numChildren; i++){
            TreeNode* param = param_list->children[i];
            if(param->type == NODE_EPSILON){
                break;
            }
            error_code_t err = is_term(param->children[0], local_tables);
            if(err != ERR_NONE){
                return err;
            }

        }

        return ERR_NONE;
    }


    // check if the number of parameters matches
    if(get_num_children(param_list) != param_count){
        
        return ERR_SEMANTIC_FUNC;
    }

    first(param_list_table);
    for(int i = 0; i < param_count; i++){
        // lets iterate through all parameters and check their validity
        TreeNode *param_tree = param_list->children[i];
        function_parameter_t *param_table = parameter_list_get_active(param_list_table);
        
        // we need to check:
        // if the parameter has a name assigned in symtable, it needs to have the same name as the one in the function call
        // typing


        char* param_label_from_table = param_table->label;
        // if NULL -- param should have an underscore
        // param - 1 or 2 children, if 1, then its name is underscore, if 2, then it has a name
        if(param_label_from_table == NULL && param_tree->numChildren != 1){
            first(param_list_table);
            
            return ERR_SEMANTIC_FUNC;
        }


        data_type_t param_table_type = param_table->data_type;
        bool param_table_nilable = param_table->nilable;
        // we need to determine if the passed parameter is identifier or expression
        // start by determining how many children it has, so we know which node contains the wanted data
        TreeNode* passed_param = param_tree->children[param_tree->numChildren - 1];
        data_type_t param_tree_type = node_type_to_data(passed_param->type);
        if(param_tree_type == -1){
            // the data could not be converted straight away, check for identifier and look it up in symtable

            if(passed_param->type == NODE_FUNCTION_CALL){
                // the parameter is a function call

                error_code_t err = semantic_func_call(passed_param, local_tables);
                if(err == ERR_NONE){
                    // the func call is valid, we need to check if the return type matches the param type

                    symtable_record_global_t *func_record = symtable_search(global_table, passed_param->children[0]->label, GLOBAL_TABLE);
                    if(func_record == NULL){
                        first(param_list_table);
                        return ERR_SEMANTIC_DEFINITION;
                    }

                    if(func_record->data->data_type != param_table_type){
                        first(param_list_table);
                        
                        return ERR_SEMANTIC_FUNC;
                    }

                }else{
                    first(param_list_table);
                    return err;
                }

            } else if(passed_param->type == NODE_IDENTIFIER){
                symtable_record_local_t *record = check_stack(local_tables, passed_param->label);
                // if there is same variable in the same subtree, we need to skip the first record we find
                if(same_symbol_assign){
                    record = get_nth_record(local_tables, passed_param->label, 1);
                    same_symbol_assign = false;
                }

                if(record == NULL){
                    symtable_record_global_t *record_global = symtable_search(global_table, passed_param->label, GLOBAL_TABLE);
                    if(record_global == NULL){
                        first(param_list_table);
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }



                    // if the data type does not match, we return error
                    if(record_global->data->data_type != param_table_type){
                        first(param_list_table);
                        
                        return ERR_SEMANTIC_FUNC;
                    }

                    // if the nilability does not match, we return error
                    if(record_global->data->nilable != param_table_nilable){
                        first(param_list_table);
                        return ERR_SEMANTIC_FUNC;
                    }

                    // if the variable is not defined, we return error
                    if(!record_global->data->defined){
                        first(param_list_table);
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                }else{

                    // if the data type does not match, we return error
                    if(record->data->data_type != param_table_type){
                        first(param_list_table);
                        
                        return ERR_SEMANTIC_FUNC;
                    }


                    // if the nilability does not match, we return error
                    if(record->data->nilable != param_table_nilable){
                        first(param_list_table);
                        return ERR_SEMANTIC_FUNC;
                    }

                    // if the variable is not defined, we return error
                    if(!record->data->defined){
                        first(param_list_table);
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                }
                    

            }




        }else{
            if(param_tree_type == DATA_NIL){
                // the passed param is nil
                // thats ok if the param in the table has nilable set to true
                if(!param_table->nilable){
                    first(param_list_table);
                    
                    return ERR_SEMANTIC_FUNC;
                }
            }else{
                // else we need to check if the types match
                if(param_table_type != param_tree_type){
                    
                    first(param_list_table);
                    return ERR_SEMANTIC_FUNC;
                }
            }

        

        }
            // if the param has a name, we need to check if they are matching
            if(param_label_from_table != NULL){
                TreeNode *param_name = param_tree->children[0];
                if(strcmp(param_label_from_table, param_name->label) != 0){
                    first(param_list_table); // we reset the list
                    return ERR_SEMANTIC_FUNC;
                }
            }
        
    
        parameter_list_next(record->data->parameters);
        
    }



    first(param_list_table);
    return ERR_NONE;

}



/**
 * @brief Runs a semantic analysis on return node(s)
 * @param node Root of the return node
 * @param local_symbtables Stack of local tables
 * @param function_name Name of the function the return statement is in
 * @returns Error code
*/
error_code_t semantic_return(TreeNode* node, Stack* local_symbtables, char* function_name){
    // this function checks if the return statement is valid
    // and if the return type matches the function return type
    // Tree:
    // child 0 - expression or function call
    
    TreeNode* ret_statement = node->children[0];
    // if the expression is empty, we need to check if the function return type is void
    data_type_t function_return_type = DATA_NONE;
    bool func_return_nilable = false;

    // lets extract information from the global table
    symtable_record_global_t *record = symtable_search(global_table, function_name, GLOBAL_TABLE);
    if(record == NULL){
        return ERR_INTERNAL;
    }

    if(record->data->symbol_type != SYM_FUNC){
        return ERR_SEMANTIC_DEFINITION;
    }

    function_return_type = record->data->data_type;
    func_return_nilable = record->data->nilable;
    
    if(ret_statement->type == NODE_FUNCTION_CALL){
         
        // we need to check if the function is in the global table
        // we need to check if the function is already defined
        // we need to check if the function return type matches the function call return type
        // we need to check if the function call parameters match the function parameters
        return semantic_func_call(ret_statement, local_symbtables);
    }else if(ret_statement->type == NODE_EXPRESSION){

        if(ret_statement->children[0]->type == NODE_EPSILON){
            // the expression is empty
            if(function_return_type != DATA_NONE){
                return ERR_SEMANTIC_RETURN;
            }
        }

        // if there is a missing return statement, where it should be (in non-void function), we return error
        if(function_return_type == DATA_NONE && ret_statement->children[0]->type != NODE_EPSILON){
            return ERR_SEMANTIC_RETURN;
        }

        

        data_type_t type = DATA_NONE;
        sem_ret = true;
        // lets check the expression the return statement contains
        error_code_t e = semantic_arithmetic_expression(ret_statement, &type, local_symbtables, true);
        sem_ret = false;

        if(e != ERR_NONE){
            return e;
        }

        if(type == DATA_NIL){
            // if it returns nil literal and the function return type is not nilable, we return error
            // otherwise everything is ok
            if(!func_return_nilable){
                return ERR_SEMANTIC_FUNC;
            }

            return ERR_NONE;
        }

        // compatibility check for double and int (int can be implicitly converted to double but only literal and not in division)
        if((function_return_type == DATA_DOUBLE && type == DATA_INT && !set_by_variable && !divison)){
            return ERR_NONE;
        }

        // if the typing does not match, we return error
        if(type != function_return_type){
            return ERR_SEMANTIC_FUNC;
        }


        // if the function return type is nilable and the expression is not nilable, we return error
        if(expression_nilable_bool && !func_return_nilable){
            return ERR_SEMANTIC_FUNC;
        }

        
    }else{
       
        // we have an error from the parser, should not happen
        return ERR_INTERNAL;
    }

    

    return ERR_NONE;

}


/**
 * @brief Runs semantic analysis on a function declaration
 * @param node Root of the function declaration tree
 * @returns Error code
*/
error_code_t semantic_func_declaration(TreeNode* node){
    // we need to check if the function is already declared in the global table
    // we also need to check the return type of function matching with the expression after return keyword

    // func_declrataion_neterminal
    // child 0 - func
    // child 1 - function_name
    // child 2 - param_list
    // child 3 - return type
    // child 4 - body

    TreeNode *function_name = node->children[1];
    TreeNode *func_return_type = node->children[3];
    TreeNode *body = node->children[4];

    bool local = false;

    char* f_name = function_name->label;
    // the table should already be in the global table, but with defined set to false
    // if the defined is true, then the function is already defined (checked by semantic) - return error
    symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
    if(record != NULL){
        // if we found function that is already in the global table, we need to check if it's already checked by semantic
        if(record->data->symbol_type == SYM_FUNC){
            if(!record->data->defined){
                // the function is not defined (has no body)
                // semantic error
                return ERR_SEMANTIC_DEFINITION;
            }


            
        }else{
            // this is most likely a symbol redefinition resulting in error
            return ERR_SEMANTIC_DEFINITION;
        }
    }else{
        // the function is not in the global table, there must be an error from the parser
        return ERR_INTERNAL;
    }

    


    
    int ret_count = return_count(body, true);

    // if the function has 0 return statements, we need to check if the return type is void
    if(ret_count == 0 && record->data->data_type != DATA_NONE){
        return ERR_SEMANTIC_FUNC;
    }
    
    return ERR_NONE;
}


/**
 * @brief Runs semantic analysis on a declaration (without assignment)
 * @param node Root of the declaration tree
 * @param local_symtables Stack of local tables
 * @returns Error code
 * @note This function is used for both variable and constant declaration
 * @note Declaration with assign is handled by semantic_assign
*/
error_code_t semantic_declaration(TreeNode* node, Stack* local_symtables){
    // not much to do here except updating the record to value nil if the declaration is nilable
    // declaration
    // child 0 - identifier
    // child 1 - type
    TreeNode* identifier = node->children[0];

    data_type_t *nil = malloc(sizeof(data_type_t));
    if(nil == NULL){
        return ERR_INTERNAL;
    }

    *nil = DATA_NIL;

    symtable_record_local_t* record = check_stack(local_symtables, identifier->label);

    if(record == NULL){
        // we did not find the identifier in the local tables, we need to check the global table
        symtable_record_global_t *record_global = symtable_search(global_table, identifier->label, GLOBAL_TABLE);
        if(record_global == NULL){
            // we did not find the identifier in the global table (which is weird, because it should be there)
            // so we throw internal error
            return ERR_INTERNAL;
        }

        if(record_global->data->nilable){

            record_global->data->value = nil;
            record_global->data->defined = true;
        }
    }else{
        // we found the identifier in the local tables
        if(record->data->nilable){
            record->data->value = nil;
            record->data->defined = true;
        }
    }



    return ERR_NONE;
}

/**
 * @brief Runs semantic analysis on an assign statement
 * @param node Root of the assign statement tree
 * @param local_tables Stack of local tables
 * @returns Error code
*/
error_code_t semantic_assign(TreeNode* node, Stack* local_tables){
    
    // assign : always 2 children
    // L-child: identifier or declaration
    // R-child: expression or function call
    TreeNode* left_child = node->children[0];
    TreeNode* right_child = node->children[1];



    data_type_t type_of_var = DATA_NONE;
    bool variable_nilable = false;

    bool declaration = false;
    bool with_type = false;

    

    TreeNode* identifier = left_child;

    if(left_child->type == NODE_DECLARATION){
        identifier = left_child->children[0];
        declaration = true;

        if(left_child->children[1]->type != NODE_EPSILON){
            with_type = true;
        }

    }

    // this is important to detect, since if there is a circular dependency, we need to get the 2nd record
    same_symbol_assign = same_identifier_in_subtrees(right_child, left_child, identifier->label) && declaration;

    // we need to check if the identifier exists in the local tables or in the global table
    // we need to check if the identifier is not read-only
    // we need to check if the identifier type matches the expression type

    symtable_record_local_t *record = check_stack(local_tables, identifier->label);
    symtable_record_global_t *record_global = NULL;
    if(record == NULL){
        // we did not find the identifier in the local tables, we need to check the global table
       record_global = symtable_search(global_table, identifier->label, GLOBAL_TABLE);
        if(record_global == NULL){
            // we did not find the identifier in the global table
            return ERR_SEMANTIC_NOT_DEFINED;
        }

        // trying to assign to constant or parameter
        symbol_type_t symbol_type = record_global->data->symbol_type;
        if((symbol_type == SYM_CONSTANT || symbol_type == SYM_PARAMETER) && !declaration){
            return ERR_SEMANTIC_OTHERS;
        }

        type_of_var = record_global->data->data_type;
        variable_nilable = record_global->data->nilable;

        // we need to validate the variable, since it can contain pointer to NIL from parser, even though it's not nilable
        if(record_global->data->value != NULL && *((data_type_t*)record_global->data->value) == DATA_NIL){
            if(record_global->data->data_type == DATA_NONE){
                // the type could not be determined from nil immediate value, so we return error
                return ERR_SEMANTIC_TYPE;
            }

            if(!variable_nilable){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }
        }


    }else{
        // we found the identifier in the local tables
        // we need to check if the identifier is not read-only
        symbol_type_t type_loc = record->data->symbol_type;
        if((type_loc == SYM_CONSTANT || type_loc == SYM_PARAMETER) && !declaration){
            return ERR_SEMANTIC_OTHERS;
        }

       

        type_of_var = record->data->data_type;
        variable_nilable = record->data->nilable;

        // we need to validate the variable, since it can contain pointer to NIL from parser, even though it's not nilable
        if(record->data->value != NULL && *((data_type_t*)record->data->value) == DATA_NIL){
            if(record->data->data_type == DATA_NONE){
                // the type could not be determined from nil immediate value, so we return error
                return ERR_SEMANTIC_TYPE;
            }

            if(!variable_nilable){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }
        }

    }

    data_type_t type_of_r_value = DATA_NONE;
    bool r_value_nilable = false;
  
    if(right_child->type == NODE_FUNCTION_CALL){
        // we need to check if the function is in the global table
        // we need to check if the function is already defined
        // we need to check if the function return type matches the function call return type
        // we need to check if the function call parameters match the function parameters
        error_code_t er = semantic_func_call(right_child, local_tables);
        if(er != ERR_NONE){
            return er;
        }

        // need to check the return type of the function call
        // we know the record exists, because we checked it in semantic_func_call
       symtable_record_global_t *fun_record = symtable_search(global_table, right_child->children[0]->label, GLOBAL_TABLE);



        // fun_record is a void function
        if(fun_record->data->data_type == DATA_NONE){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }


        // Int -> Int - valid
        // Int -> Int? - valid
        // Int? -> Int - invalid
        // Int? -> Int? - valid
        // if the function return type is nilable, the variable type must be nilable as well

        type_of_r_value = fun_record->data->data_type;
        r_value_nilable = fun_record->data->nilable;

    }else if(right_child->type == NODE_EXPRESSION){
        error_code_t er = semantic_arithmetic_expression(right_child, &type_of_r_value, local_tables, true);
        if(er != ERR_NONE){
            return er;
        }

        r_value_nilable = expression_nilable_bool;        

    }

    if(declaration && !with_type){
        // its declaration without type specified (let x = 5)
        if(record != NULL){
            record->data->data_type = type_of_r_value;
            record->data->nilable = r_value_nilable;
        }else if(record_global != NULL){
            record_global->data->data_type = type_of_r_value;
            record_global->data->nilable = r_value_nilable;
        }

    }else{
        // its an assign or declaration with type

        if(type_of_r_value == DATA_NIL){
            if(!variable_nilable){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            return ERR_NONE;
        }else{
            // the r_value is not nil, we update the record
            if(record != NULL){
                record->data->value = NULL;
            }
            else if(record_global != NULL){
                record_global->data->value = NULL;
            }
            
        }


        if(r_value_nilable && !variable_nilable){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

        if(!is_assign_compatible(type_of_var, type_of_r_value, expression_immediate, coal_found, variable_nilable, r_value_nilable)){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

    }


    
    
    return ERR_NONE;
}


/**
 * @brief Runs semantic analysis on a guard let expression
 * @param node Root of the guard let expression
 * @param local_tables Stack of local tables
 * @param guarded_let_key Pointer to the key of the guarded let variable
 * @returns Error code
 * @note Guard let block is meant to be used in if and while statements
 * @note if(let x)...else{}
*/
error_code_t semantic_guard_let(TreeNode* node, Stack* local_tables, char** guarded_let_key){
    TreeNode* identifier = node->children[0];

    // if the variable was not declared, its an error
    // if the variable was declared, but not as read-only, its an error

    char* identifier_name = identifier->label;
    if(identifier_name == NULL){
        return ERR_INTERNAL;
    }

    *guarded_let_key = identifier->label;

    return ERR_NONE;
}


/**
 * @brief Finds the body end node in a tree, recursively
 * @param node Root of the tree
 * @returns Pointer to the body end node
 * @note This function is used during marking the end of guard let block
*/
TreeNode* find_body_end(TreeNode* node){
    // this function finds the body end node
    // body_end
    // child 0 - body
    // child 1 - body_end
    // child 2 - epsilon

    if(node->type == NODE_BODY_END){
        return node;
    }

    if(node->numChildren == 0){
        return NULL;
    }

    TreeNode* body_end = NULL;
    for(int i = 0; i < node->numChildren; i++){
        body_end = find_body_end(node->children[i]);
        if(body_end != NULL){
            return body_end;
        }
    }

    return NULL;
}


/**
 * @brief Runs semantic analysis on an if statement
 * @param node Root of the if statement
 * @param local_tables Stack of local tables
 * @returns Error code
*/
error_code_t semantic_if_statement(TreeNode* node, Stack* local_tables){
    // if_statement
    // child 0 - expression or guard expression
    // child 1 - body (if true block)
    // child 2 - else body (if false block)

    TreeNode* expression = node->children[0];
    TreeNode* body = node->children[1];
    TreeNode* else_body = node->children[2];

    // if there is expression, we need to check it for semantic errors
    if(expression->type == NODE_EXPRESSION){
        return semantic_relation_expression(expression, local_tables);
    }else if(expression->type == NODE_GUARD_LET){
    // if there is guard let block we need to check it for semantic errors
        char* guarded_let_key = NULL;
        error_code_t e =  semantic_guard_let(expression, local_tables, &guarded_let_key);
        if(e != ERR_NONE){
            return e;
        }

        TreeNode* body_end = find_body_end(else_body);
        if(body_end == NULL){
            return ERR_INTERNAL;
        }

        // mark the end of the body with the key of the guard let variable
        body_end->label = guarded_let_key;
    }

    return ERR_NONE;
}


/**
 * @brief Runs semantic analysis on a while statement
 * @param node Root of the while statement
 * @param local_tables Stack of local tables
 * @returns Error code
*/
error_code_t semantic_while_statement(TreeNode* node, Stack* local_tables){
    // while_statement
    // child 0 - expression
    // child 1 - body

    TreeNode* expression = node->children[0];
    TreeNode* body = node->children[1];

    // there can only be expression, so we need to check it for semantic errors
    if(expression->type == NODE_EXPRESSION){
        
        return semantic_relation_expression(expression, local_tables);
    }

    

    return ERR_NONE;
}






/**
 * @brief Main function for semantic analysis
 * @note Calls helper functions depending on the node type
*/
error_code_t semantic(TreeNode *node){
    if(node == NULL){
        return ERR_INTERNAL;
    }
    NodeType type = node->type;

    switch (type)
    {
    case NODE_FUNCTION_CALL:
        return semantic_func_call(node, stack_of_local_tables);

    case NODE_DECLARATION_FUNCTION:
        return semantic_func_declaration(node);

    case NODE_ASSIGN:
        return semantic_assign(node, stack_of_local_tables);

    case NODE_IF_STATEMENT:
        return semantic_if_statement(node, stack_of_local_tables);

    case NODE_DECLARATION:
        return semantic_declaration(node, stack_of_local_tables);

    case NODE_WHILE:
        return semantic_while_statement(node, stack_of_local_tables);
    case NODE_RETURN:
        return semantic_return(node, stack_of_local_tables, current_function_name);


    default:
        break;
    }

    return ERR_NONE;

}

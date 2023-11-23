#include "header_files/parser.h"
#include "header_files/stack.h"
#include "header_files/code_gen.h"
#include "header_files/semantic.h"


// Stack *stack_of_local_tables = NULL;
bool in_body_neterminal = false;
bool set_by_variable = false;
bool next_identifier_unwrapped = false;
bool scan_for_coal = false;
bool coal_found = false;
static bool expression_nilable_bool = false;

bool is_neterminal(TreeNode *node){
    return !node->terminal;
}

int return_count(TreeNode *node){
    static int count = 0;
    for(int i = 0; i < node->numChildren; i++){
        if(node->children[i]->type == NODE_RETURN){
            count++;
        }
        return_count(node->children[i]);
    }
    return count;
}

void get_all_returns(TreeNode *node, TreeNode** returns){
    // function assumes that the returns array is initialized
    static int index = 0;
    if(returns == NULL){
        return;
    }

    if(node == NULL){
        return;
    }

    if(node->type == NODE_RETURN){
        returns[index++] = node;
    }

    for(int i = 0; i < node->numChildren; i++){
        get_all_returns(node->children[i], returns);
    }
    
}


size_t get_num_children(TreeNode *node){
    size_t count = 0;
    for(int i = 0; i < node->numChildren; i++){
        if(node->children[i] != NULL && node->children[i]->type != NODE_EPSILON){
            count++;
        }
    }
    return count;
}

bool is_binary_arithmetic(NodeType type){
    return type == NODE_OPERATOR_ADD || type == NODE_OPERATOR_SUB || type == NODE_OPERATOR_MUL || type == NODE_OPERATOR_DIV || type == NODE_OPERATOR_NIL_COALESCING;
}

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

bool is_immediate_operand(NodeType type){
    return type == NODE_INT || type == NODE_DOUBLE || type == NODE_STRING || type == NODE_NIL;
}

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

// this function assumes that the node is an immediate operand
bool is_datatype_compatible(data_type_t type1, data_type_t type2, bool coal_found){

    if(type2 == DATA_NIL && coal_found){
        // nil cannot be on the right side of coallescing operator
        return false;
    }
    if(type1 == DATA_NIL){
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

bool types_compatible_relation(data_type_t type1, data_type_t type2, bool type1_immediate, bool type2_immediate){
    if (type1_immediate && type2_immediate) {
    // if both sides are immediate, double and int are compatible
        return (type1 == DATA_INT && type2 == DATA_DOUBLE) || (type1 == DATA_DOUBLE && type2 == DATA_INT) || (type1 == type2);
    } else if (type1_immediate || type2_immediate) {
    // Either type1 or type2 is immediate
        return (type1_immediate == DATA_INT && (type2 == DATA_INT || type2 == DATA_DOUBLE)) ||
            (type2_immediate == DATA_INT && (type1 == DATA_INT || type1 == DATA_DOUBLE));
    } else {
        // Neither type1 nor type2 is immediate
        return type1 == type2;
    }
}




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

    if(reset) first_run = true;

    if(first_run){
        if(node->numChildren == 1 && node->children[0]->type == NODE_NIL){
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
            next_identifier_unwrapped = true;
        }
            
        first_run = false;
    }

    

    for(int i = 0; i < node->numChildren; i++){
        TreeNode* child = node->children[i];
        
        if(child->type == NODE_EXPRESSION){
            if(child->numChildren >= 2 && child->children[1]->type == NODE_OPERATOR_UNARY){
                next_identifier_unwrapped = true;
            }
            
            error_code_t er = semantic_arithmetic_expression(child, data_type, local_tables, false);
            if(er != ERR_NONE){
                return er;
            }
        }
        else{
            if(is_immediate_operand(child->type)){

                if(*data_type != DATA_NONE){
                    data_type_t child_type = node_type_to_data(child->type);
                    if(!is_datatype_compatible(*data_type, child_type, coal_found)){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                    if((*data_type == DATA_DOUBLE && child_type == DATA_INT)){
                       continue;
                    }else if(*data_type == DATA_INT && child_type == DATA_DOUBLE && set_by_variable){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }
                }
                
                set_by_variable = false;
                *data_type = node_type_to_data(child->type);

                return ERR_NONE;

            }else if(child->type == NODE_IDENTIFIER){
            
                symtable_record_local_t* record = check_stack(local_tables, child->label);

                data_type_t var_data_type = DATA_NONE;
                bool var_nilable = false;
                data_type_t *is_nil = NULL;
                if(record == NULL){
                    symtable_record_global_t* glob_record = symtable_search(global_table, child->label, GLOBAL_TABLE);

                    if(glob_record == NULL){
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                    if(!glob_record->data->defined){
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                    if(coal_found){
                        glob_record->data->nilable = false;
                    }

                    var_data_type = glob_record->data->data_type;
                    var_nilable = glob_record->data->nilable;
                    is_nil = glob_record->data->value;

                    

                }else{
                    if(!record->data->defined){
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }

                    if(coal_found){
                        record->data->nilable = false;
                    }

                    var_data_type = record->data->data_type;
                    var_nilable = record->data->nilable;
                    is_nil = record->data->value;
                }

                
                if(is_nil != NULL && *is_nil == DATA_NIL){
                    // variable has value nil
                    if(!var_nilable){
                        // todo idk what error to return
                        return ERR_SEMANTIC_OTHERS;
                    }

                    if(!next_identifier_unwrapped && !coal_found)
                        var_data_type = DATA_NIL;
                    

                    next_identifier_unwrapped = false;
                }

                // if we use unwrapping operator on something that is not nilable, we return error
                if(next_identifier_unwrapped && !var_nilable){ 
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }

                if(var_nilable){
                    if(!next_identifier_unwrapped){
                        expression_nilable_bool = true;
                        scan_for_coal = true;
                    }else{

                        expression_nilable_bool = false;
                        next_identifier_unwrapped = false;
                    }

                }
                

                if(*data_type != DATA_NONE){
                    if(!is_datatype_compatible(*data_type, var_data_type, coal_found)){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                    if((*data_type == DATA_DOUBLE && var_data_type == DATA_INT)){
                        // this covers either 3.2 - x, where x is Int
                       return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }
                    
                    if(*data_type != var_data_type){
                        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                    }

                }
                *data_type = var_data_type;
                set_by_variable = true;

                return ERR_NONE;

            }else if(is_binary_arithmetic(child->type)){
                // TODO: nil ?? nil -> error 7
                // TODO: x (nil) coallescing nil -> error 7 (currently throws 4)
                if(scan_for_coal && child->type != NODE_OPERATOR_NIL_COALESCING){
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }else{
                    scan_for_coal = false;

                }

                if(*data_type == DATA_STRING && child->type != NODE_OPERATOR_ADD){
                    return ERR_SEMANTIC_TYPE_COMPATIBILITY;
                }
                
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

    

    if(scan_for_coal){
        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
    }

    return ERR_NONE;
}


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


error_code_t semantic_relation_expression(TreeNode* node, bool *result, Stack* local_tables){
    TreeNode* expression = node;
    if(node->children[0]->type == NODE_LEFT_PARENTHESIS){
        expression = node->children[1];
    }
    


    TreeNode* l_expression = expression->children[0];
    TreeNode* r_expression = expression->children[2];
    TreeNode* operator = expression->children[1];

    if(l_expression == NULL || r_expression == NULL || operator == NULL){
        return ERR_SEMANTIC_TYPE_COMPATIBILITY;
    }

    

    data_type_t l_type = DATA_NONE;
    data_type_t r_type = DATA_NONE;

    
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
            if(r_expression_nilable || l_expression_nilable){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            if(!types_compatible_relation(l_type, r_type, l_immediate, r_immediate)){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            break;
        

        case NODE_OPERATOR_EQUAL:
        case NODE_OPERATOR_NEQ:
            
            if(!types_compatible_relation(l_type, r_type, l_immediate, r_immediate)){
                return ERR_SEMANTIC_TYPE_COMPATIBILITY;
            }

            break;
        default:
            return ERR_INTERNAL;
    }

    return ERR_NONE;
}

bool is_term(NodeType type){
    switch (type)
    {
    case NODE_INT:
    case NODE_DOUBLE:
    case NODE_STRING:
    case NODE_IDENTIFIER:
    case NODE_NIL:
        return true;
    
    default:
        return false;
        break;
    }
}

error_code_t semantic_func_call(TreeNode* node, Stack* local_tables){
    TreeNode *function_name = node->children[0];
    TreeNode *param_list = node->children[1];
    char* f_name = function_name->label;


    symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
    if(record == NULL){
        return ERR_SEMANTIC_DEFINITION;
    }

    parameter_list_t *param_list_table = record->data->parameters;
    size_t param_count = parameter_list_get_size(param_list_table);

    if(param_count == SIZE_MAX){
        // the function has infinite parameters, so we need to check if the passed parameters are terms
        // if they are not, we return error

        for(int i = 0; i < param_list->numChildren; i++){
            TreeNode* param = param_list->children[i];
            if(!is_term(param->children[0]->type)){
                return ERR_SEMANTIC_FUNC;
            }
        }

        return ERR_NONE;
    }


    if(get_num_children(param_list) != param_count){
        return ERR_SEMANTIC_FUNC;
    }

    first(param_list_table);
    for(int i = 0; i < param_count; i++){
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
        // todo we need to determine if the passed parameter is identifier or expression
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
                if(record == NULL){
                    symtable_record_global_t *record_global = symtable_search(global_table, passed_param->label, GLOBAL_TABLE);
                    if(record_global == NULL){
                        first(param_list_table);
                        return ERR_SEMANTIC_NOT_DEFINED;
                    }


                    
                    if(record_global->data->data_type != param_table_type){
                        first(param_list_table);
                    
                        return ERR_SEMANTIC_FUNC;
                    }
                }else{

                    printf("record global: %d\n", record->data->data_type);
                    printf("param_table_type: %d\n", param_table_type);

                    if(record->data->data_type != param_table_type){
                        first(param_list_table);
                        return ERR_SEMANTIC_FUNC;
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

error_code_t semantic_return(TreeNode* node, Stack* local_symbtables, data_type_t function_return_type, bool func_return_nilable){
    // this function checks if the return statement is valid
    // and if the return type matches the function return type
    // Tree:
    // child 0 - expression or function call
    
    TreeNode* ret_statement = node->children[0];
    // if the expression is empty, we need to check if the function return type is void
    
    if(ret_statement->type == NODE_FUNCTION_CALL){
         
        // we need to check if the function is in the global table
        // we need to check if the function is already defined
        // we need to check if the function return type matches the function call return type
        // we need to check if the function call parameters match the function parameters
        return semantic_func_call(ret_statement, local_symbtables);
    }else if(ret_statement->type == NODE_EXPRESSION){
        // todo semantic expression and resolve typing
        if(ret_statement->children[0]->type == NODE_EPSILON){
            // the expression is empty
            if(function_return_type != DATA_NONE){
                return ERR_SEMANTIC_RETURN;
            }
        }
        if(function_return_type == DATA_NONE && ret_statement->children[0]->type != NODE_EPSILON){
            return ERR_SEMANTIC_RETURN;
        }

        

        // todo resolve typing and expression semantic
        data_type_t type = DATA_NONE;
        error_code_t e = semantic_arithmetic_expression(ret_statement, &type, local_symbtables, true);
        if(e != ERR_NONE){
            return e;
        }

        if(type == DATA_NIL){
            if(!func_return_nilable){
                return ERR_SEMANTIC_FUNC;
            }

            return ERR_NONE;
        }

        if((function_return_type == DATA_DOUBLE && type == DATA_INT && !set_by_variable)){
            return ERR_NONE;
        }

        if(type != function_return_type){
            return ERR_SEMANTIC_FUNC;
        }

        if(expression_nilable_bool && !func_return_nilable){
            return ERR_SEMANTIC_FUNC;
        }

        



    }else{
       
        // we have an error
        return ERR_INTERNAL;
    }

    

    return ERR_NONE;

}



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
    // todo we found record that is not a function
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

    
    // the body has no return statement and the return type is not void - semantic error
    // TreeNode* return_node = NULL;
    // if(!has_return(body, &return_node) && record->data->data_type != DATA_NONE){
    //     return ERR_SEMANTIC_FUNC;
    // }
    int ret_count = return_count(body);
    TreeNode** returns = malloc(sizeof(TreeNode*) * ret_count);
    get_all_returns(body, returns);

    for(int i = 0; i < ret_count; i++){
        TreeNode* return_node = returns[i];
        if(return_node != NULL){
            error_code_t er = semantic_return(return_node, stack_of_local_tables, record->data->data_type, record->data->nilable);
            if(er != ERR_NONE){
                free(returns);
                return er;
            }
        }
    }

    if(ret_count == 0 && record->data->data_type != DATA_NONE){
        free(returns);
        return ERR_SEMANTIC_FUNC;
    }

    free(returns);
    return ERR_NONE;
}


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

        // trying to assign to constant
        if(record_global->data->symbol_type == SYM_CONSTANT && !declaration){
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
        if(record->data->symbol_type == SYM_CONSTANT && !declaration){
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

        // if(fun_record->data->nilable && !variable_nilable){
        //     return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        // }

        // if(fun_record->data->data_type != type_of_var){
        //     return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        // }

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
        }


        if(r_value_nilable && !variable_nilable){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

        if(!is_datatype_compatible(type_of_var, type_of_r_value, coal_found)){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

    }


    
    
    return ERR_NONE;
}


error_code_t semantic_guard_let(TreeNode* node, Stack* local_tables, char** guarded_let_key){
    TreeNode* identifier = node->children[0];

    // if the variable was not declared, its an error
    // if the variable was declared, but not as read-only, its an error

    char* identifier_name = identifier->label;
    if(identifier_name == NULL){
        return ERR_INTERNAL;
    }

    symtable_record_local_t* record = check_stack(local_tables, identifier_name);
    if(record == NULL){
        // we did not find the identifier in the local tables, we need to check the global table
        symtable_record_global_t *record_global = symtable_search(global_table, identifier_name, GLOBAL_TABLE);
        if(record_global == NULL){
            // we did not find the identifier in the global table
            return ERR_SEMANTIC_NOT_DEFINED;
        }

        if(record_global->data->symbol_type != SYM_CONSTANT){
            return ERR_SEMANTIC_OTHERS;
        }

        if(!record_global->data->defined){
            return ERR_SEMANTIC_NOT_DEFINED;
        }

        if(!record_global->data->nilable){
            return ERR_SEMANTIC_OTHERS;
        }

        record_global->data->nilable = false;

    }else{
        // we found the identifier in the local tables
        // we need to check if the identifier is not read-only
        if(record->data->symbol_type != SYM_CONSTANT){
            return ERR_SEMANTIC_OTHERS;
        }

        if(!record->data->defined){
            return ERR_SEMANTIC_NOT_DEFINED;
        }

        if(!record->data->nilable){
            return ERR_SEMANTIC_OTHERS;
        }
        record->data->nilable = false;
    }

    *guarded_let_key = identifier->label;

    return ERR_NONE;
}

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

error_code_t semantic_if_statement(TreeNode* node, Stack* local_tables){
    // if_statement
    // child 0 - expression or guard expression
    // child 1 - body (if true block)
    // child 2 - else body (if false block)

    TreeNode* expression = node->children[0];
    TreeNode* body = node->children[1];
    TreeNode* else_body = node->children[2];

    if(expression->type == NODE_EXPRESSION){
        return semantic_relation_expression(expression, NULL, local_tables);
    }else if(expression->type == NODE_GUARD_LET){
        char* guarded_let_key = NULL;
        error_code_t e =  semantic_guard_let(expression, local_tables, &guarded_let_key);
        if(e != ERR_NONE){
            return e;
        }

        TreeNode* body_end = find_body_end(else_body);
        if(body_end == NULL){
            return ERR_INTERNAL;
        }

        body_end->label = guarded_let_key;
    }
    // todo: guard_let, semantic body or else_body bool for not nilable variable in the guard let expression

    return ERR_NONE;
}

error_code_t semantic_while_statement(TreeNode* node, Stack* local_tables){
    // while_statement
    // child 0 - expression
    // child 1 - body

    TreeNode* expression = node->children[0];
    TreeNode* body = node->children[1];

    

    if(expression->type == NODE_EXPRESSION){
        
        return semantic_relation_expression(expression, NULL, local_tables);
    }

    

    return ERR_NONE;
}













error_code_t semantic(TreeNode *node){
    if(node == NULL){
        return ERR_INTERNAL;
    }
    NodeType type = node->type;
    
    if(type == NODE_BODY_END && in_body_neterminal){
        // pop table from the stack
        in_body_neterminal = false;
        //stack_pop(stack_of_local_tables);

    }
    
    if(type == NODE_FUNCTION_CALL){
        return semantic_func_call(node, stack_of_local_tables);
    } else if(type == NODE_BODY){
        // we hit a noterminal with a body, 
        // we need to push it to the stack
       
        // todo: we need to make sure to pop the table from the stack when we leave the block
        in_body_neterminal = true;

    }
    else if(type == NODE_DECLARATION_FUNCTION){
        return semantic_func_declaration(node);
    }else if(type == NODE_ASSIGN){
        return semantic_assign(node, stack_of_local_tables);
    }else if(type == NODE_IF_STATEMENT){
       return semantic_if_statement(node, stack_of_local_tables);
    }
    else if(type == NODE_DECLARATION){
        return semantic_declaration(node, stack_of_local_tables);
    }else if(type == NODE_WHILE){
        
        return semantic_while_statement(node, stack_of_local_tables);
    }

    

    return ERR_NONE;

}



// int main(void){
//     // global table variable is defined here
//     stack_of_local_tables = stack_init(STACK_INIT_CAPACITY);

//     error_code_t error = ERR_INTERNAL;


//     file = fopen("test.txt", "r");
//     if (file == NULL)
//     {
//         return error;
//     }

//     TreeNode *startNeterminal = createNewNode(NULL, NODE_PROGRAM, false);

//     if (parse(startNeterminal, false, false))
//     {
//         error = ERR_NONE;
//     }else{
//         return ERR_SYNTAX_ANALYSIS;
//     }

//     print_global_table(global_table);
//     printTree(startNeterminal);
//     error = semantic(startNeterminal->children[0]);

//     dispose(startNeterminal);

//     if (fclose(file) == EOF)
//     {
//         error = ERR_INTERNAL;
//     }
//     printf("%d\n", error);
//     return error;

// }


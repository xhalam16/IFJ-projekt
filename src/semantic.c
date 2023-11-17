#include "header_files/parser.h"
#include "header_files/stack.h"
#include "header_files/semantic.h"


// Stack *stack_of_local_tables = NULL;
bool in_body_neterminal = false;

bool is_neterminal(TreeNode *node){
    return !node->terminal;
}

bool has_return(TreeNode *node, TreeNode** return_node){
    // this functions accepts a node that is a body
    // it checks if the body has a return statement

    if(node == NULL){
        return false;
    }

    if(node->type == NODE_RETURN){
        *return_node = node;
        return true;
    }

    for(int i = 0; i < node->numChildren; i++){
        if(has_return(node->children[i], return_node)){
            return true;
        }
    }




    return false;
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



error_code_t semantic_func_call(TreeNode* node){
        TreeNode *function_name = node->children[0];
        TreeNode *param_list = node->children[1];
        char* f_name = function_name->label;

        symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
        if(record == NULL){
            return ERR_SEMANTIC_DEFINITION;
        }
        parameter_list_t *param_list_table = record->data->parameters;
        size_t param_count = parameter_list_get_size(param_list_table);


        if(get_num_children(param_list) != param_count){
            return ERR_SEMANTIC_FUNC;
        }

        for(int i = 0; i < param_count; i++){
            TreeNode *param_tree = param_list->children[i];
            function_parameter_t *param_table = param_list_table->active;

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

                    error_code_t err = semantic_func_call(passed_param);
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

                }

            }
    
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



        if(param_list_table != NULL)
           first(param_list_table);
        return ERR_NONE;
}

error_code_t semantic_return(TreeNode* node, Stack* local_symbtables, data_type_t function_return_type){
    // this function checks if the return statement is valid
    // and if the return type matches the function return type
    // Tree:
    // child 0 - return keyword
    // child 1 - expression
    // child 0 of expression - identifier or literal or epsilon (if empty) or function call
    
    TreeNode* expression = node->children[1];
    TreeNode* ret_statement = expression->children[0];

    // if the expression is empty, we need to check if the function return type is void
    if(ret_statement->type == NODE_EPSILON){
        if(function_return_type != DATA_NONE){
            return ERR_SEMANTIC_RETURN;
        }
    }
    
    if(ret_statement->type == NODE_IDENTIFIER){
        // we need to check if the identifier is in the local tables (that are on the stack) or in the global table

        for(int i = 0; i < stack_size(local_symbtables); i++){
            Stack_Frame* frame = stack_get(local_symbtables, i);
            local_symtable* table = (local_symtable*)frame->data;

            symtable_record_local_t *record = symtable_search(table, ret_statement->label, LOCAL_TABLE);

            if(record != NULL){
                // we found the identifier in the local table
                // we need to check if the identifier is initialized
                if(!record->data->defined){
                    return ERR_SEMANTIC_NOT_DEFINED;
                }
                // we need to check if the identifier is a variable
                if(record->data->symbol_type != SYM_VAR){
                    return ERR_SEMANTIC_FUNC;
                }
                // we need to check if the identifier type matches the function return type
                if(record->data->data_type != function_return_type){
                    return ERR_SEMANTIC_FUNC;
                }

                return ERR_NONE;
            }
        }

        // we did not find it in the local tables, we need to check the global table
        symtable_record_global_t *record = symtable_search(global_table, ret_statement->label, GLOBAL_TABLE);
        if(record == NULL){
            // we did not find the identifier in the global table
            return ERR_SEMANTIC_DEFINITION;
        }

        if(record->data->data_type != function_return_type){
            return ERR_SEMANTIC_FUNC;
        }
    }

    if(ret_statement->type == NODE_FUNCTION_CALL){
        // we need to check if the function is in the global table
        // we need to check if the function is already defined
        // we need to check if the function return type matches the function call return type
        // we need to check if the function call parameters match the function parameters
        return semantic_func_call(ret_statement);
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

    char* f_name = function_name->label;
    // the table should already be in the global table, but with defined set to false
    // if the defined is true, then the function is already defined (checked by semantic) - return error
    symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
    // todo we found record that is not a function
    if(record != NULL){
        // if we found function that is already in the global table, we need to check if it's already checked by semantic
        if(record->data->symbol_type == SYM_FUNC){
            bool * checked_by_sem = (bool*)record->data->value;
            if(*checked_by_sem){
                // if the function is already checked by semantic, it's already defined, meaning this is a redefinition => error
                return ERR_SEMANTIC_DEFINITION;
            }else{
                // else we mark the function as checked by semantic
                bool checked = true;
                record->data->value = &checked;
            }
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
    TreeNode* return_node = NULL;
    if(!has_return(body, &return_node) && record->data->data_type != DATA_NONE){
        return ERR_SEMANTIC_RETURN;
    }

    // TODO
    // check if the return type of the function matches the return type of the return statement

    if(return_node != NULL)
        return semantic_return(return_node, stack_of_local_tables, record->data->data_type);



    return ERR_NONE;
}

symtable_record_local_t* check_stack(Stack* local_tables, char* identifier){
    for(int i = 0; i < stack_size(local_tables); i++){
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

error_code_t semantic_assign(TreeNode* node, Stack* local_tables){

    // assign : always 2 children
    // L-child: identifier or declaration
    // R-child: expression or function call
    TreeNode* left_child = node->children[0];
    TreeNode* right_child = node->children[1];
    data_type_t type_of_var = DATA_NONE;
    bool variable_nilable = false;



    if(left_child->type == NODE_DECLARATION){
        // todo call semantic_declaration
    } else if(left_child->type == NODE_IDENTIFIER){
        // we need to check if the identifier exists in the local tables or in the global table
        // we need to check if the identifier is not read-only
        // we need to check if the identifier type matches the expression type

        symtable_record_local_t *record = check_stack(local_tables, left_child->label);
        if(record == NULL){
            // we did not find the identifier in the local tables, we need to check the global table
            symtable_record_global_t *record_global = symtable_search(global_table, left_child->label, GLOBAL_TABLE);
            if(record_global == NULL){
                // we did not find the identifier in the global table
                return ERR_SEMANTIC_NOT_DEFINED;
            }

            if(record_global->data->symbol_type == SYM_CONSTANT){
                return ERR_SEMANTIC_OTHERS;
            }

            type_of_var = record_global->data->data_type;
            variable_nilable = record_global->data->nilable;

        }else{
            // we found the identifier in the local tables
            // we need to check if the identifier is not read-only
            if(record->data->symbol_type == SYM_CONSTANT){
                return ERR_SEMANTIC_OTHERS;
            }
            type_of_var = record->data->data_type;
            variable_nilable = record->data->nilable;
        }

    }else{
        // we have an error
        return ERR_INTERNAL;
    }

    if(right_child->type == NODE_FUNCTION_CALL){
        // we need to check if the function is in the global table
        // we need to check if the function is already defined
        // we need to check if the function return type matches the function call return type
        // we need to check if the function call parameters match the function parameters
        error_code_t er = semantic_func_call(right_child);
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

        // todo: resolve typing, check if the function call return type matches the variable type
        // Int -> Int - valid
        // Int -> Int? - valid
        // Int? -> Int - invalid
        // Int? -> Int? - valid
        // if the function return type is nilable, the variable type must be nilable as well

        if(fun_record->data->nilable && !variable_nilable){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }

        if(fun_record->data->data_type != type_of_var){
            return ERR_SEMANTIC_TYPE_COMPATIBILITY;
        }


    }else if(right_child->type == NODE_EXPRESSION){
        // todo semantic expression and resolve typing
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
        return semantic_func_call(node);
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


#include "header_files/parser.h"
#include "header_files/stack.h"


Stack *stack_of_local_tables = NULL;

bool is_neterminal(TreeNode *node){
    return !node->terminal;
}

error_code_t semantic_func_call(TreeNode* node){
    TreeNode *function_name = node->children[0];
        TreeNode *param_list = node->children[1];
        char* f_name = function_name->label;

        symtable_record_global_t *record = symtable_search(global_table, f_name, GLOBAL_TABLE);
        if(record == NULL){
            return ERR_SEMANTIC_DEFINITION;
        }

        size_t param_count = record->data->parameters->size;


        if(param_list->numChildren != param_count){
            return ERR_SEMANTIC_FUNC;
        }

        for(int i = 0; i < param_count; i++){
            TreeNode *param_tree = param_list->children[i];
            function_parameter_t *param_table = record->data->parameters->active;

            // we need to check:
            // if the parameter has a name assigned in symtable, it needs to have the same name as the one in the function call
            // typing


            char* param_label_from_table = param_table->label;
            // if NULL -- param should have an underscore
            // param - 1 or 2 children, if 1, then its name is underscore, if 2, then it has a name
            if(param_label_from_table == NULL && param_tree->numChildren != 1){
                return ERR_SEMANTIC_FUNC;
            }


            data_type_t param_table_type = param_table->data_type;
            // todo we need to determine if the passed parameter is identifier or expression
            // start by determining how many children it has, so we know which node contains the wanted data
            TreeNode* passed_param = param_tree->children[param_tree->numChildren - 1];
            data_type_t param_tree_type = node_type_to_data(passed_param->type);
            if(param_tree_type == -1){
                // the data could not be converted straight away, check for identifier and look it up in symtable
              

            }
    
            if(param_table_type != param_tree_type){
                
                return ERR_SEMANTIC_FUNC;
            }


            // if the param has a name, we need to check if they are matching
            if(param_label_from_table != NULL){
               TreeNode *param_name = param_tree->children[0];
                if(strcmp(param_label_from_table, param_name->label) != 0){
                    return ERR_SEMANTIC_FUNC;
                }
            }

            parameter_list_next(record->data->parameters);
            
        }





        return ERR_NONE;
}

error_code_t semantic(TreeNode *node){
    if(node == NULL){
        return ERR_INTERNAL;
    }
    NodeType type = node->type;

    if(type == NODE_FUNCTION_CALL){
        return semantic_func_call(node);
    }



    return ERR_NONE;

}



int main(void){
    // global table variable is defined here
    stack_of_local_tables = stack_init(STACK_INIT_CAPACITY);

    error_code_t error = ERR_INTERNAL;


    file = fopen("test.txt", "r");
    if (file == NULL)
    {
        return error;
    }

    TreeNode *startNeterminal = createNewNode(NULL, NODE_PROGRAM, false);

    if (parse(startNeterminal, false, false, false))
    {
        error = ERR_NONE;
    }

    //print_global_table(global_table);
    //printTree(startNeterminal);
    error = semantic(startNeterminal->children[1]);

    dispose(startNeterminal);

    if (fclose(file) == EOF)
    {
        error = ERR_INTERNAL;
    }
    printf("%d\n", error);
    return error;

}


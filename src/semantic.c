#include "header_files/parser.h"

bool is_neterminal(TreeNode *node){
    return !node->terminal;
}

error_code_t semantic(TreeNode *node){

    if(node == NULL){
        return ERR_INTERNAL;
    }
    NodeType type = node->type;

    if(type == NODE_FUNCTION_CALL){
        
    }



    return ERR_NONE;

}



int main(void){
    // global table variable is defined here


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
     printf("%d\n", error);
    return error;

}


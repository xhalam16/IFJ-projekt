#include "header_files/parser.h"

int main(void)
{
    stack_of_local_tables = stack_init(STACK_INIT_CAPACITY);
    if (stack_of_local_tables == NULL)
    {
        error = ERR_INTERNAL;
        return error;
    }

    error = ERR_SYNTAX_ANALYSIS;
    // file = stdin;
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

    // print_global_table(global_table);

    // print_stack(stack_of_local_tables);
    bool ar[10] = {true};

    // printTree(startNeterminal, ar, 0, 0);
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



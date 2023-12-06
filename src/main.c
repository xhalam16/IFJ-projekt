/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: main.c
 * Datum: 24. 11. 2023
 * Autor: Šimon Motl, xmotls00
 *        Richard Juřica, xjuric31
 *        Marek Halamka, xhalam16
 */

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
    file = stdin;
    // file = fopen("test.txt", "r");
    // if (file == NULL)
    // {
    //     error = ERR_INTERNAL;
    //     return error;
    // }

    TreeNode *startNeterminal = createNewNode(NULL, NODE_PROGRAM, false);

    if (parse(startNeterminal))
    {
        error = ERR_NONE;
    }

    dispose(startNeterminal);
    symtable_free(global_table, GLOBAL_TABLE);
    stack_free(stack_of_local_tables);

    // if (fclose(file) == EOF)
    // {
    //     error = ERR_INTERNAL;
    // }

    // printf("%d\n", error);
    return error;
}



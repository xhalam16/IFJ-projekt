/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: semantic.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#pragma once

#include "parser.h"
#include "stack.h"

symtable_record_local_t* check_stack(Stack* local_tables, char* identifier);

error_code_t semantic(TreeNode *node);



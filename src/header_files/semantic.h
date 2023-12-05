/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: semantic.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#pragma once

#include "parser.h"
#include "stack.h"

/**
 * @brief Looks for a record containing the given identifier in the given stack of local tables
 * @param local_tables The stack of local tables to look in
 * @param identifier The identifier to look for
 * @return The record containing the identifier if it was found, NULL otherwise
*/
symtable_record_local_t* check_stack(Stack* local_tables, char* identifier);


/**
 * @brief Main function of the semantic analysis, calls the other functions
 * @param node The node of AST to analyze
 * @return Corresponding error code
*/
error_code_t semantic(TreeNode *node);



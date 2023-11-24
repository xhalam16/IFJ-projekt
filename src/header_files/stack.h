/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: stack.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STACK_INIT_CAPACITY 16
#define STACK_ERROR -1
#define STACK_SUCCESS 0


/**
 * @brief A frame in the stack
*/
typedef struct stack_frame {
    void* data;
} Stack_Frame;


/**
 * @brief A stack of Stack_Frames
 * @param frames The frames in the stack
 * @param size The size of the stack
 * @param capacity The capacity of the stack
 * @param top The index of the top element in the stack (top is -1 if the stack is empty)
*/
typedef struct stack {
    Stack_Frame* frames;
    int size;
    int capacity;
    int top;
} Stack;


/**
 * @brief Initializes a stack with a given capacity
 * @param capacity The capacity of the stack
*/
Stack* stack_init(int capacity);


/**
 * @brief Resizes the stack to a new capacity
 * @param stack The stack to resize
 * @param new_cap The new capacity of the stack
 * @return STACK_ERROR if the stack could not be resized, STACK_SUCCESS otherwise
*/
int resize_stack(Stack* stack, size_t new_cap);


/**
 * @brief Pushes a new element to the top of the stack
 * @param stack The stack to push to
 * @param data The data to push to the stack
 * @return STACK_ERROR if the stack could not be resized, STACK_SUCCESS otherwise
*/
int stack_push(Stack* stack, void* data);


/**
 * @brief Returns the top element of the stack
 * @param stack The stack to get the top element from
 * @return The frame at the top of the stack
*/
Stack_Frame* stack_top(Stack* stack);


/**
 * @brief Pops the top element of the stack
 * @param stack The stack to pop from
*/
void stack_pop(Stack* stack);

/**
 * @brief Checks if the stack is full
 * @param stack The stack to check
 * @return true if the stack is full, false otherwise
*/
bool stack_is_full(Stack* stack);

/**
 * @brief Frees the stack
 * @param stack The stack to free
*/
void stack_free(Stack* stack);

bool stack_is_empty(Stack* stack);

Stack_Frame* stack_get(Stack* stack, int index);

int stack_size(Stack* stack);

void stack_empty(Stack* stack);


#include "header_files/stack.h"

Stack* stack_init(int capacity){
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if(stack == NULL){
        return NULL;
    }

    stack->capacity = capacity;
    stack->top = -1;
    stack->frames = malloc(sizeof(Stack_Frame) * capacity);
    return stack;
}

int resize_stack(Stack* stack, size_t new_cap){
    if(stack == NULL){
        return STACK_ERROR;
    }

    stack->frames = realloc(stack->frames, sizeof(Stack_Frame) * new_cap);
    if(stack->frames == NULL){
        return STACK_ERROR;
    }
    stack->capacity = new_cap;

    return STACK_SUCCESS;
}

bool is_full(Stack* stack){
    if(stack == NULL){
        return false;
    }

    return stack->top == stack->capacity - 1;
}

int stack_push(Stack* stack, void* data){
    if(stack == NULL){
        return STACK_ERROR;
    }

    if(is_full(stack)){
        int res = resize_stack(stack, stack->capacity * 2);
        if(res == STACK_ERROR){
            return STACK_ERROR;
        }
    }
    Stack_Frame frame;
    frame.data = data;

    stack->top++;
    stack->frames[stack->top] = frame;

    return STACK_SUCCESS;
}

Stack_Frame* stack_top(Stack* stack){
    if(stack == NULL){
        return NULL;
    }

    if(stack->top != -1){
        return &(stack->frames[stack->top]);
    }

    return NULL;
}

void stack_pop(Stack* stack){
    if(stack == NULL){
        return;
    }

    if(stack->top == -1){
        return;
    }
    stack->top--;
}

void stack_free(Stack* stack){
    if(stack == NULL){
        return;
    }

    free(stack->frames);
    free(stack);
}
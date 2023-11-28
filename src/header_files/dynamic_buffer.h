/* 
 * Projekt: Překladač jazyka IFJ23
 * Soubor: dynamic_buffer.h
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */


#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ERR_CODE_OK 0
#define ERR_CODE_ALLOC 1
#define BUFFER_INIT_CAPACITY 32

typedef struct dynamic_buffer {
    char* buffer;
    size_t size;
    size_t capacity;
} DynamicBuffer;

int init_buffer(DynamicBuffer* buffer, size_t capacity);
int resize_buffer(DynamicBuffer* buffer, size_t new_capacity);
int buffer_append_string(DynamicBuffer* buffer, const char* data);
int buffer_append_char(DynamicBuffer* buffer, char data);
void buffer_clear(DynamicBuffer* buffer);
int buffer_insert_char_beggining(DynamicBuffer* buffer, char data);
bool buffer_equals_string(DynamicBuffer* buffer, const char* string);

void free_buffer(DynamicBuffer* buffer);

int move_buffer(char** dest, DynamicBuffer* src);

int move_buffer_to_buffer(DynamicBuffer* dest, DynamicBuffer* src);




#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ERR_CODE_OK 0
#define ERR_CODE_ALLOC 1
#define BUFFER_INIT_CAPACITY 16

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

void move_buffer(char* dest, DynamicBuffer* src);


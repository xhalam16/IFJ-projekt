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
#include <math.h>


/**
 * @brief Macros used by the dynamic buffer
 * @param ERR_CODE_OK The error code for successful operations
 * @param ERR_CODE_ALLOC The error code for failed allocations
 * @param BUFFER_INIT_CAPACITY The initial capacity of the buffer
*/
#define ERR_CODE_OK 0
#define ERR_CODE_ALLOC 1
#define BUFFER_INIT_CAPACITY 32

/**
 * @brief Stucture defining a dynamic buffer
 * @param buffer The buffer itself
 * @param size The size of the buffer, without '\0'
 * @param capacity The maximum capacity of the buffer
 * @typedef DynamicBuffer
 */
typedef struct dynamic_buffer {
    char* buffer;
    size_t size; // without '\0'
    size_t capacity;
} DynamicBuffer;


/**
 * @brief Initializes a dynamic buffer with a given capacity
 * @param buffer The buffer to initialize
 * @param capacity The capacity of the buffer
 * @return ERR_CODE_OK if the buffer was initialized successfully, ERR_CODE_ALLOC otherwise
 * @note The function expects a valid pointer to a DynamicBuffer
*/
int init_buffer(DynamicBuffer* buffer, size_t capacity);


/**
 * @brief Resizes the buffer to a new capacity
 * @param buffer The buffer to resize
 * @param new_capacity The new capacity of the buffer
 * @return ERR_CODE_OK if the buffer was resized successfully, ERR_CODE_ALLOC otherwise
*/
int resize_buffer(DynamicBuffer* buffer, size_t new_capacity);


/**
 * @brief Appends a string to the buffer
 * @param buffer The buffer to append to
 * @param data The string to append
 * @return ERR_CODE_OK if the string was appended successfully, ERR_CODE_ALLOC otherwise
*/
int buffer_append_string(DynamicBuffer* buffer, const char* data);


/**
 * @brief Appends a character to the buffer
 * @param buffer The buffer to append to
 * @param data The character to append
 * @return ERR_CODE_OK if the character was appended successfully, ERR_CODE_ALLOC otherwise
*/
int buffer_append_char(DynamicBuffer* buffer, char data);


/**
 * @brief Clears the buffer
 * @param buffer The buffer to clear
 * @warning The buffer is not freed, only its size is set to 0
*/
void buffer_clear(DynamicBuffer* buffer);


/**
 * @brief Inserts a character to the buffer at the beginning
 * @param buffer The buffer to insert to
 * @param data The character to be inserted
 * @return ERR_CODE_OK if the character was inserted successfully, ERR_CODE_ALLOC otherwise
*/
int buffer_insert_char_beggining(DynamicBuffer* buffer, char data);

/**
 * @brief Compares the buffer to a string and returns whether they are equal
 * @param buffer The buffer to compare
 * @param string The string to compare
 * @return true if the buffer and the string are equal, false otherwise
*/
bool buffer_equals_string(DynamicBuffer* buffer, const char* string);


/**
 * @brief Moves the contents of the source buffer to the destination
 * @param dest The destination
 * @param src The source buffer
 * @return ERR_CODE_OK if the buffer was moved successfully, ERR_CODE_ALLOC otherwise
 * @note If the destination is NULL, it is automatically allocated, if it's not NULL, but its capacity is not enough, it is resized 
*/
int move_buffer(char** dest, DynamicBuffer* src);


/**
 * @brief Moves the contents of the source buffer to the destination buffer
 * @param dest The destination buffer
 * @param src The source buffer
 * @return ERR_CODE_OK if the buffer was moved successfully, ERR_CODE_ALLOC otherwise
 * @note The destination buffer is resized if its capacity is not enough
*/
int move_buffer_to_buffer(DynamicBuffer* dest, DynamicBuffer* src);


/**
 * @brief Prints the buffer into the given file
 * @param buffer The buffer to print
 * @param file The file to print into
*/
void buffer_print_into_file(DynamicBuffer *buffer, FILE *file);



/**
 * @brief Frees the buffer
 * @param buffer The buffer to free
*/
void free_buffer(DynamicBuffer* buffer);
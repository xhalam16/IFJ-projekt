/*
 * Projekt: Překladač jazyka IFJ23
 * Soubor: dynamic_buffer.c
 * Datum: 24. 11. 2023
 * Autor: Marek Halamka, xhalam16
 */

#include "header_files/dynamic_buffer.h"

void terminate_buffer(DynamicBuffer* buffer){
    buffer->buffer[buffer->size] = '\0';
}


int init_buffer(DynamicBuffer* buffer, size_t capacity){
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->buffer = calloc(capacity, sizeof(char)); // zatemňování: -5 bodů standardní ceník už dlouhá léta

    
    if(buffer->buffer == NULL){
        return ERR_CODE_ALLOC;
    }

    terminate_buffer(buffer);

    return ERR_CODE_OK;
}

int resize_buffer(DynamicBuffer *buffer, size_t new_capacity)
{
    char *new_buffer = realloc(buffer->buffer, new_capacity);
    if (new_buffer == NULL)
    {
        return ERR_CODE_ALLOC;
    }

    buffer->buffer = new_buffer;
    buffer->capacity = new_capacity;

    return ERR_CODE_OK;
}

void free_buffer(DynamicBuffer *buffer)
{
    if(buffer->buffer != NULL)
        free(buffer->buffer);

    if(buffer != NULL)
        free(buffer);
}

int buffer_append_char(DynamicBuffer *buffer, char data)
{
    if (buffer->size + 1 > buffer->capacity)
    {
        if (resize_buffer(buffer, buffer->capacity * 2) != ERR_CODE_OK)
        {
            return ERR_CODE_ALLOC;
        }
    }

    buffer->buffer[buffer->size] = data;
    buffer->size += 1;

    //buffer->buffer[buffer->size] = '\0';
    terminate_buffer(buffer);

    return ERR_CODE_OK;
}

int buffer_append_string(DynamicBuffer *buffer, const char *data)
{
    int data_size = strlen(data) + 1;
        
    if(buffer->size + data_size > buffer->capacity){
        int quotient = ceil((buffer->size + data_size) / buffer->capacity);
        if(resize_buffer(buffer, buffer->capacity * quotient) != ERR_CODE_OK){
            return ERR_CODE_ALLOC;
        }
    }
   
    
    // data are already null terminated
    memcpy(buffer->buffer + buffer->size, data, data_size);

    buffer->size += data_size - 1;
    return ERR_CODE_OK;
}

void buffer_clear(DynamicBuffer *buffer)
{

    strcpy(buffer->buffer, "");

    buffer->size = 0;

}

int buffer_insert_char_beggining(DynamicBuffer *buffer, char data)
{
    if (buffer->size + 1 > buffer->capacity)
    {
        if (resize_buffer(buffer, buffer->capacity * 2) != ERR_CODE_OK)
        {
            return ERR_CODE_ALLOC;
        }
    }

    memmove(buffer->buffer + 1, buffer->buffer, buffer->size);
    buffer->buffer[0] = data;
    buffer->size += 1;

    return ERR_CODE_OK;
}

bool buffer_equals_string(DynamicBuffer *buffer, const char *string)
{
    return strcmp(buffer->buffer, string) == 0;
}

int move_buffer(char **dest, DynamicBuffer *src)
{

    if (*dest == NULL)
    {
        *dest = malloc(src->size * sizeof(char) + 1);
        if (*dest == NULL)
        {
            return ERR_CODE_ALLOC;
        }
    }
    else
    {
        if (strlen(*dest) < src->size)
        {
            *dest = realloc(*dest, src->size * sizeof(char));

            if (*dest == NULL)
            {
                return ERR_CODE_ALLOC;
            }
        }
    }
    strcpy(*dest, src->buffer);
    // buffer_clear(src);
    return ERR_CODE_OK;
}

int move_buffer_to_buffer(DynamicBuffer *dest, DynamicBuffer *src)
{

    if (dest->capacity < src->size)
    {
        if (resize_buffer(dest, src->size) != ERR_CODE_OK)
        {
            return ERR_CODE_ALLOC;
        }
    }

    memcpy(dest->buffer, src->buffer, src->size + 1);

    dest->size = src->size;
    return ERR_CODE_OK;
}


void buffer_print_into_file(DynamicBuffer *buffer, FILE *file)
{
    for (int i = 0; i < buffer->size; i++)
    {
        fprintf(file, "%c", buffer->buffer[i]);
    }
}




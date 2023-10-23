#include "header_files/dynamic_buffer.h" 

int init_buffer(DynamicBuffer* buffer, size_t capacity){
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->buffer = malloc(capacity * sizeof(char));
    if(buffer->buffer == NULL){
        return ERR_CODE_ALLOC;
    }



    return ERR_CODE_OK;
}


int resize_buffer(DynamicBuffer* buffer, size_t new_capacity){
    char* new_buffer = realloc(buffer->buffer, new_capacity);
    if(new_buffer == NULL){
        return ERR_CODE_ALLOC;
    }

    buffer->buffer = new_buffer;
    buffer->capacity = new_capacity;

    return ERR_CODE_OK;
}


void free_buffer(DynamicBuffer* buffer){
    free(buffer->buffer);
    buffer->buffer = NULL;
    buffer->capacity = 0;
    buffer->size = 0;
}

int buffer_append_char(DynamicBuffer* buffer, char data){
    if(buffer->size + 1 > buffer->capacity){
        if(resize_buffer(buffer, buffer->capacity * 2) != ERR_CODE_OK){
            return ERR_CODE_ALLOC;
        }
    }

    buffer->buffer[buffer->size] = data;
    buffer->size += 1;

    buffer->buffer[buffer->size] = '\0';

    return ERR_CODE_OK;
}

int buffer_append_string(DynamicBuffer* buffer, const char* data){
    int data_size = strlen(data) + 1;

    if(buffer->size + data_size > buffer->capacity){
        if(resize_buffer(buffer, buffer->capacity * 2) != ERR_CODE_OK){
            return ERR_CODE_ALLOC;
        }
    }

    // data are already null terminated
    memcpy(buffer->buffer + buffer->size, data, data_size);
    buffer->size += data_size;
    return ERR_CODE_OK;
}


void buffer_clear(DynamicBuffer* buffer){
    buffer->size = 0;
}

int buffer_insert_char_beggining(DynamicBuffer* buffer, char data){
    if(buffer->size + 1 > buffer->capacity){
        if(resize_buffer(buffer, buffer->capacity * 2) != ERR_CODE_OK){
            return ERR_CODE_ALLOC;
        }
    }

    memmove(buffer->buffer + 1, buffer->buffer, buffer->size);
    buffer->buffer[0] = data;
    buffer->size += 1;

    return ERR_CODE_OK;
}

bool buffer_equals_string(DynamicBuffer* buffer, const char* string){
    return strcmp(buffer->buffer, string) == 0;
}
#include "header_files/symtable.h"


//http://www.cse.yorku.ca/~oz/hash.html - varianta sdbm
size_t hash_function(const char *str)
{
    uint32_t h = 0; // musí mít 32 bitů
    const unsigned char *p;
    for (p = (const unsigned char *)str; *p != '\0'; p++)
        h = 65599 * h + *p;
    return h;
}


global_symtable* create_global_symtable(const size_t capacity){
    global_symtable *table = malloc(sizeof(global_symtable));
    if(table == NULL){
        return NULL;
    }

    table->capacity = capacity;
    table->size = 0;
    table->records = malloc(sizeof(symtable_record_global_t *) * capacity);
    if(table->records == NULL){
        free(table);
        return NULL;
    }

    // initialize all records to NULL
    for(int i = 0; i < capacity; i++){
        table->records[i] = NULL;
    }

    return table;
}

local_symtable* create_local_symtable(const size_t capacity){
    local_symtable *table = malloc(sizeof(local_symtable));
    if(table == NULL){
        return NULL;
    }

    table->capacity = capacity;
    table->size = 0;
    table->records = malloc(sizeof(symtable_record_local_t *) * capacity);
    if(table->records == NULL){
        free(table);
        return NULL;
    }
    // initialize all records to NULL
    for(int i = 0; i < capacity; i++){
        table->records[i] = NULL;
    }

    return table;
}

int resize_table(void* table, table_type_t type ,size_t new_cap){
    if(table == NULL){
        return ERR_CODE_ST_FAIL;
    }

    if(type == LOCAL_TABLE){
        local_symtable *local_table = (local_symtable *)table;
        local_table->records = realloc(local_table->records, sizeof(symtable_record_local_t *) * new_cap);
        if(local_table->records == NULL){
            return ERR_CODE_ST_FAIL;
        }
        local_table->capacity = new_cap;

        // initialize all NEW records to NULL
        for(int i = local_table->size; i < new_cap; i++){
            local_table->records[i] = NULL;
        }

        return ERR_CODE_ST_OK;
    }else if(type == GLOBAL_TABLE){
        global_symtable *global_table = (global_symtable *)table;
        global_table->records = realloc(global_table->records, sizeof(symtable_record_global_t *) * new_cap);
        if(global_table->records == NULL){
            return ERR_CODE_ST_FAIL;
        }
        global_table->capacity = new_cap;

        // initialize all NEW records to NULL
        for(int i = global_table->size; i < new_cap; i++){
            global_table->records[i] = NULL;
        }

        return ERR_CODE_ST_OK;
    }
    

    return ERR_CODE_ST_FAIL;

}

size_t get_capacity(void *table){
    if(table == NULL){
        return 0;
    }

    return ((global_symtable *)table)->capacity;
}

size_t get_size(void *table){
    if(table == NULL){
        return 0;
    }

    return ((global_symtable *)table)->size;
}

symtable_local_data_t* create_local_data(symbol_type_t symbol_type, data_type_t data_type, bool nilable, bool defined, void *value){
    symtable_local_data_t *data = malloc(sizeof(symtable_local_data_t));
    if(data == NULL){
        return NULL;
    }

    data->symbol_type = symbol_type;
    data->data_type = data_type;
    data->nilable = nilable;
    data->defined = defined;
    data->value = value;

    return data;
}

symtable_global_data_t* create_global_data(symbol_type_t symbol_type, data_type_t data_type, bool nilable, bool defined, void *value, parameter_list_t *parameters){
    symtable_global_data_t *data = malloc(sizeof(symtable_global_data_t));
    if(data == NULL){
        return NULL;
    }

    data->symbol_type = symbol_type;
    data->data_type = data_type;
    data->nilable = nilable;
    data->defined = defined;
    data->value = value;
    data->parameters = parameters;

    return data;
}

bool is_full(void* table){
    if(table == NULL){
        return false;
    }

    return ((global_symtable *)table)->size == ((global_symtable *)table)->capacity;
}

bool index_occupied(void *table, int index){
    if(table == NULL){
        return false;
    }

    return ((global_symtable *)table)->records[index] != NULL;
}

int find_next_free_index(void *table, size_t index, table_type_t type){
    if(table == NULL){
        return ERR_CODE_ST_FAIL;
    }

    if(is_full(table)){
        return INDEX_NOT_FOUND;
    }

    size_t starting_index = index;

    while(index_occupied(table, index)){
        index++;
        if(index >= get_capacity(table)){
            index = 0;
        }

        // this means that we have checked all indexes and there is no free index
        if(starting_index == index){
            return INDEX_NOT_FOUND;
        }
    }

    return index;
}


int get_index(void *table, char *key, table_type_t table_type){

    // this function should probably first look at the index returned by hash_function(key) % ST_SIZE
    // if not found, the function should iterate through the table until it finds the key
    // since our table implements open addressing, we can't just return NULL if the index is occupied
    // returns pointer to data if found, NULL otherwise

    if(table == NULL || key == NULL){
        return INDEX_NOT_FOUND;
    }


    size_t index = hash_function(key) % get_capacity(table);

    if(table_type == GLOBAL_TABLE){
        symtable_record_global_t *record = ((global_symtable *)table)->records[index];
        if(record != NULL){
            if(strcmp(record->key, key) == 0){
                return index;
            }
        }

    }else if(table_type == LOCAL_TABLE){
        symtable_record_local_t *record = ((local_symtable *)table)->records[index];
        if(record != NULL){
            if(strcmp(record->key, key) == 0){
                return index;
            }
        }
    }else{
        return INDEX_NOT_FOUND;
    }


    // in both cases, the record was not found at the index returned by hash_function(key) % ST_SIZE
    // we need to iterate through the table until we find the record
    for(int i = 0; i < get_capacity(table); i++){
        if(table_type == GLOBAL_TABLE){
            symtable_record_global_t *record = ((global_symtable *)table)->records[i];
            if(record != NULL){
                if(strcmp(record->key, key) == 0){
                    return i;
                }
            }
        }else if(table_type == LOCAL_TABLE){
            symtable_record_local_t *record = ((local_symtable *)table)->records[i];
            if(record != NULL){
                if(strcmp(record->key, key) == 0){
                    return i;
                }
            }
        }else{
            return INDEX_NOT_FOUND;
        }
    }
    
    
    return INDEX_NOT_FOUND;

}

void* symtable_search(void* table, char* key, table_type_t table_type){
    if(table == NULL || key == NULL){
        return NULL;
    }

    int index = get_index(table, key, table_type);
    if(index == INDEX_NOT_FOUND){
        return NULL;
    }

    if(table_type == GLOBAL_TABLE){
        return ((global_symtable *)table)->records[index];
    }else if(table_type == LOCAL_TABLE){
        return ((local_symtable *)table)->records[index];
    }else{
        return NULL;
    }

}


int symtable_insert(void *table, char *key, void *data, table_type_t type){

    if(table == NULL || key == NULL || data == NULL){
        return ERR_CODE_ST_FAIL;
    }
    size_t index = hash_function(key) % get_capacity(table);

    if(index_occupied(table, index)){
        int new_i = find_next_free_index(table, index, type);
        if(new_i == INDEX_NOT_FOUND){ 
            // this means that the table is full, we need to resize it
            int res = resize_table(table, type, get_capacity(table) * 2);
            if(res != ERR_CODE_ST_OK){
                return ERR_CODE_ST_FAIL;
            }
            // we can assume that the table is not full anymore
            new_i = find_next_free_index(table, index, type);

        }

        index = (size_t)new_i;
    }

    if(symtable_search(table, key, type) != NULL){
        return ERR_CODE_ST_FAIL;
    }
    


    if(type == GLOBAL_TABLE){
        // data of type symtable_global_data_t are expected
        symtable_global_data_t *data_global = (symtable_global_data_t *)data;
        global_symtable* global_table = (global_symtable *) table;
        
        symtable_record_global_t *record = malloc(sizeof(symtable_record_global_t));
        if(record == NULL){
            return ERR_CODE_ST_FAIL;
        }

        record->data = data_global;
        record->key = key;

        global_table->records[index] = record;
        global_table->size++;

    }else if(type == LOCAL_TABLE){
        // data of type symtable_local_data_t are expected
        symtable_local_data_t *data_local = (symtable_local_data_t *)data;
        local_symtable* local_table = (local_symtable *) table;
        // data are already expected to be properly initialized
        symtable_record_local_t *record = malloc(sizeof(symtable_record_local_t));
        if(record == NULL){
            return ERR_CODE_ST_FAIL;
        }
        record->data = data_local;
        record->key = key;

        local_table->records[index] = record;
        local_table->size++;

    }else{
        return ERR_CODE_ST_FAIL;
    }

    return ERR_CODE_ST_OK;
}

void symtable_delete(void *table, char *key, table_type_t type){
    int index_of_record = get_index(table, key, type);
    if(type == LOCAL_TABLE){
        symtable_record_local_t *record = symtable_search(table, key, type);
        if(record == NULL){
            return;
        }
        free(record->data);
        free(record);

        ((local_symtable *)table)->records[index_of_record] = NULL;
        ((local_symtable *)table)->size--;
    }else if(type == GLOBAL_TABLE){
        symtable_record_global_t *record = symtable_search(table, key, type);
        if(record == NULL){
            return;
        }
        free(record->data);
        free(record);

        ((global_symtable *)table)->records[index_of_record] = NULL;
        ((global_symtable *)table)->size--;

    }
}

void symtable_clear(void* table, table_type_t type){
    if(type == GLOBAL_TABLE){
        for(int i = 0; i < get_capacity(table); i++){
            symtable_record_global_t *record = ((global_symtable *)table)->records[i];
            if(record != NULL){
                parameter_list_free(record->data->parameters);
                if(record->data->value != NULL){
                    free(record->data->value);
                }

                free(record->data);
                free(record);
            }
        }
        ((global_symtable *)table)->size = 0;
    } else if(type == LOCAL_TABLE){
        for(int i = 0; i < get_capacity(table); i++){
            symtable_record_local_t *record = ((local_symtable *)table)->records[i];
            if(record != NULL){
                if(record->data->value != NULL){
                    free(record->data->value);
                }
                free(record->data);
                free(record);
            }
        }
        ((local_symtable *)table)->size = 0;
    }
}


void symtable_free(void *table, table_type_t type){
    if(table == NULL){
        return;
    }

    symtable_clear(table, type);
    if(type == GLOBAL_TABLE){
        free(((global_symtable *)table)->records);
    }else if(type == LOCAL_TABLE){
        free(((local_symtable *)table)->records);
    }
    
    free(table);
}




//********************PARAMETER LIST*************************
void parameter_list_init(parameter_list_t *list){
    list->size = 0;
    list->first = NULL;
    list->active = NULL;
}

bool parameter_list_empty(parameter_list_t *list){
    return list->size == 0;
}

bool parameter_list_active(parameter_list_t *list){
    return list->active != NULL;
}



void parameter_list_next(parameter_list_t *list){
    if(!parameter_list_active(list)){
        return;
    }
    list->active = list->active->next;
}

void parameter_list_insert(parameter_list_t *list, function_parameter_t *parameter){
    if(parameter_list_empty(list)){
        list->first = parameter;
        list->active = parameter;
    }else{
        list->active->next = parameter;
        parameter_list_next(list);
    }
    list->size++;
}

function_parameter_t *parameter_list_get_active(parameter_list_t *list){
    if(!parameter_list_active(list)){
        return NULL;
    }

    return list->active;
}

void first(parameter_list_t *list){
    if(list == NULL){
        return;
    }

    list->active = list->first;
}


void parameter_list_free(parameter_list_t *list){
    if(list == NULL){
        return;
    }

    first(list);
    while(parameter_list_active(list)){
        function_parameter_t *param = parameter_list_get_active(list);
        free_param(param);
        parameter_list_next(list);
    }
    //free(list);
}


void init_param(function_parameter_t *param){
    param->next = NULL;
    param->name = NULL;
    param->data_type = DATA_NONE;
    param->label = NULL;
    param->nilable = false;
}

void free_param(function_parameter_t *param){
    if(param == NULL){
        return;
    }
    // free(param->name);
    // free(param->label);
    free(param);
}


size_t parameter_list_get_size(parameter_list_t *list){
    if(list == NULL){
        return 0;
    }

    return list->size;
}



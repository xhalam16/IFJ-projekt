#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
// symtable - vytvářena parserem, používána v semantické analýze


#define ST_LOCAL_INIT_SIZE 50
#define ST_GLOBAL_INIT_SIZE 100
#define ERR_CODE_ST_OK 0
#define ERR_CODE_ST_FAIL 1
#define INDEX_NOT_FOUND -1

// ***************************************** STRUCTURES USED BY SYMTABLES ******************************************
typedef struct symtable_record symtable_record_t;

/**
 * @brief Enum representing type of table
*/
typedef enum table_type {
    GLOBAL_TABLE,
    LOCAL_TABLE
} table_type_t;


/**
 * @brief Enum representing type of symbol
*/
typedef enum symbol_type {
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAMETER,
    SYM_CONSTANT,
    SYM_NONE
} symbol_type_t;


/**
 * @brief Enum representing type of data
*/
typedef enum data_types {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_NIL,
    DATA_NONE
} data_type_t;


/**
 * @brief Structure representing parameter of function (used by global table) 
 * @param name - name of parameter
 * @param label - label of parameter (in named parameters of function)
 * @param data_type - type of data (DATA_INT, DATA_DOUBLE, DATA_STRING, DATA_NIL)
 * @param nilable - true if data is nilable, false otherwise
 * @param next - pointer to next parameter
*/
typedef struct function_parameter {
    char *name;
    char *label; 
    data_type_t data_type;
    bool nilable;

    struct function_parameter *next;
} function_parameter_t;



// ***************************************** LOCAL SYMTABLE ******************************************

/**
 * @brief Structure representing data of local table
 * @param symbol_type - type of symbol (SYM_VAR, SYM_FUNC, SYM_PARAMETER, SYM_CONSTANT)
 * @param data_type - type of data (DATA_INT, DATA_DOUBLE, DATA_STRING, DATA_NIL)
 * @param nilable - true if data is nilable, false otherwise
 * @param defined - true if data is defined, false otherwise
 * @param value - pointer to value of any data type
*/
typedef struct symtable_local_data {
    symbol_type_t symbol_type;
    data_type_t data_type;
    bool nilable;

    bool defined;

    void * value;

} symtable_local_data_t;


/**
 * @brief Structure representing record of local table
 * @param key - key of record (usually name of variable or function)
 * @param data - pointer to data of record
*/
typedef struct symtable_record_local {
    char *key;
    symtable_local_data_t* data;
} symtable_record_local_t;


/**
 * @brief Structure representing local symtable
 * @param records - array of pointers to records
 * @param size - number of records in table
 * @param capacity - capacity of table
*/
typedef struct l_symtable{
    symtable_record_local_t **records;
    size_t size;
    size_t capacity;
} local_symtable;


// ***************************************** GLOBAL SYMTABLE ******************************************

/**
 * @brief Structure representing data of global table
 * @param symbol_type - type of symbol (SYM_VAR, SYM_FUNC, SYM_PARAMETER, SYM_CONSTANT)
 * @param data_type - type of data (DATA_INT, DATA_DOUBLE, DATA_STRING, DATA_NIL)
 * @param nilable - true if data is nilable, false otherwise
 * @param defined - true if data is defined, false otherwise
 * @param value - pointer to value of any data type
 * @param local_table - pointer to local table (NULL if symbol_type != SYM_FUNC)
 * @param parameters - pointer to parameters of function (NULL if symbol_type != SYM_FUNC)
*/
typedef struct symtable_global_data{
    symbol_type_t symbol_type;
    data_type_t data_type; 
    bool nilable;

    bool defined;
    void * value;

    local_symtable *local_table;
    function_parameter_t *parameters;

} symtable_global_data_t;


/**
 * @brief Structure representing record of global table
 * @param key - key of record (usually name of variable or function)
 * @param data - pointer to data of record
*/
typedef struct symtable_record_global {
    char *key;
    symtable_global_data_t* data;
} symtable_record_global_t;



/**
 * @brief Structure representing global symtable
 * @param records - array of pointers to records
 * @param size - number of records in table
 * @param capacity - capacity of table
*/
typedef struct {
    symtable_record_global_t **records;
    size_t size;
    size_t capacity;
} global_symtable;





// ***************************************** FUNCTIONS ******************************************
// All functions returning int return ERR_CODE_ST_OK on success, otherwise ERR_CODE_ST_FAIL
// Functions accept table in form of void pointer, because of the different types of tables
// Type of table needs to be specified in parameter table_type_t type

/**
 * Hash function for symtable (license: //http://www.cse.yorku.ca/~oz/hash.html - varianta sdbm)
 * @param const key - key of record
 * @return index corresponding to key (!resulting index is not necessarily in range of table!)
 */
size_t hash_function(const char *key);

/**
 * Creates global symtable with capacity of capacity
 * Returns pointer to global_symtable on success, NULL on failure
 * @param capacity - initial capacity of the table
 * @return pointer to global_symtable on success, NULL on failure
 */
global_symtable* create_global_symtable(const size_t capacity);


/**
    * Creates local symtable with capacity of capacity
    * Returns pointer to local_symtable on success, NULL on failure
    * @param capacity - initial capacity of the table
    * @return pointer to local_symtable on success, NULL on failure
*/
local_symtable* create_local_symtable(const size_t capacity);

/**
 * Resizes table to capacity of new_cap
 * Returns ERR_CODE_ST_OK on success, ERR_CODE_ST_FAIL on failure
 * @param table - pointer to table to be resized
 * @param type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 * @param new_cap - new capacity of the table
 * @return ERR_CODE_ST_OK on success, ERR_CODE_ST_FAIL on failure
 */
int resize_table(void* table, table_type_t type, size_t new_cap);


/**
 * Returns capacity of table
 * @param table - pointer to table
 * @return capacity of table
 */
size_t get_capacity(void *table);

/**
 * Returns size of table
 * @param table - pointer to table
 * @return size of table
 */
size_t get_size(void *table);


/**
 * Returns true if table is full, false otherwise
 * @param table - pointer to table
 * @return true if table is full, false otherwise
 */
bool is_full(void *table);


/**
 * Creates data for local table
 * @param symbol_type - type of symbol (SYM_VAR, SYM_FUNC, SYM_PARAMETER, SYM_CONSTANT)
 * @param data_type - type of data (DATA_INT, DATA_DOUBLE, DATA_STRING, DATA_NIL)
 * @param nilable - true if data is nilable, false otherwise
 * @param defined - true if data is defined, false otherwise
 * @param value - pointer to value of data
 * @return pointer to symtable_local_data_t on success, NULL on failure
 */
symtable_local_data_t* create_local_data(symbol_type_t symbol_type, data_type_t data_type, bool nilable, bool defined, void *value);

/**
 * Creates data for global table
 * @param symbol_type - type of symbol (SYM_VAR, SYM_FUNC, SYM_PARAMETER, SYM_CONSTANT)
 * @param data_type - type of data (DATA_INT, DATA_DOUBLE, DATA_STRING, DATA_NIL)
 * @param nilable - true if data is nilable, false otherwise
 * @param defined - true if data is defined, false otherwise
 * @param value - pointer to value of data
 * @param local_table - pointer to local table (NULL if symbol_type != SYM_FUNC)
 * @param parameters - pointer to parameters of function (NULL if symbol_type != SYM_FUNC)
 * @return pointer to symtable_local_data_t on success, NULL on failure
 */
symtable_global_data_t* create_global_data(symbol_type_t symbol_type, data_type_t data_type, bool nilable, bool defined, void *value, local_symtable *local_table, function_parameter_t *parameters);


/**
 * Gets index of record by key
 * @param table - pointer to table
 * @param key - key of record
 * @param table_type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 * @return index of record on success, INDEX_NOT_FOUND on failure
 */
int get_index(void *table, char *key, table_type_t table_type);


/**
 * Returns true if index is occupied, false otherwise
 * @param table - pointer to table
 * @param index - index of record
*/
bool index_occupied(void *table, int index);




/**
 * Returns next free index in table starting from starting_index
 * Since symtable is implemented as hash table with OPEN ADDRESSING
 * @param table - pointer to table
 * @param starting_index - index to start searching from
 * @param type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 * @return next free index in table or INDEX_NOT_FOUND if table is full, or ERR_CODE_ST_FAIL on failure
*/
int find_next_free_index(void *table, size_t starting_index, table_type_t type);




/**
 * Inserts record into table
 * Function expects valid data in parameter data. Data are casted in the function depending on type of table
 * This function automatically resizes table if it is full
 * @param table - pointer to table
 * @param key - key of record
 * @param data - data of record (data are casted in the function)
 * @param type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 * @return ERR_CODE_ST_OK on success, ERR_CODE_ST_FAIL on failure
 */
int symtable_insert(void *table, char *key, void *data, table_type_t type);


/**
 * Searches for record in table
 * @param table - pointer to table
 * @param key - key of record
 * @param type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 * @return pointer to record on success, NULL on failure (!pointer should be casted to needed type!)
*/
void *symtable_search(void *table, char *key, table_type_t table_type);



/**
 * Deletes record from table
 * @param table - pointer to table
 * @param key - key of record
 * @param type - type of table (GLOBAL_TABLE or LOCAL_TABLE)
 */
void symtable_delete(void *table, char *key, table_type_t type);


/**
 * Frees symtable pointed to by table
 * @param table - pointer to table
 */
void symtable_free(void *table);
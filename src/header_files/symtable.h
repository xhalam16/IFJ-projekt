#include <stdbool.h>
// symtable - vytvářena parserem, používána v semantické analýze


#define ST_SIZE 100
#define ERR_CODE_ST_OK 0
#define ERR_CODE_ST_FAIL 1

// ***************************************** STRUCTURES USED BY SYMTABLES ******************************************
typedef struct symtable_record symtable_record_t;

// distinguish between global and local table
typedef enum table_type {
    GLOBAL_TABLE,
    LOCAL_TABLE
} table_type_t;

// distinguish between variable, function, parameter and constant
typedef enum symbol_type {
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAMETER,
    SYM_CONSTANT,
    SYM_NONE
} symbol_type_t;

typedef enum data_types {
    DATA_INT,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_NIL,
    DATA_NONE
} data_type_t;

typedef struct function_parameter {
    char *name;
    char *label; 
    data_type_t data_type;
    bool nilable;

    struct function_parameter *next;
} function_parameter_t;


// data for local table
typedef struct symtable_local_data {
    symbol_type_t symbol_type;
    data_type_t data_type;
    bool nilable;

    bool defined;

    void * value;

} symtable_local_data_t;

// data for global table
typedef struct symtable_global_data{
    symbol_type_t symbol_type;
    data_type_t data_type; // if symbol_type == SYM_FUNC, data_type is return type
    bool nilable;

    bool defined;
    void * value;

    local_symtable *local_table; // NULL if symbol_type != SYM_FUNC
    function_parameter_t *parameters; // NULL if symbol_type != SYM_FUNC

} symtable_global_data_t;


typedef struct symtable_record_local {
    char *key;
    symtable_local_data_t data;
} symtable_record_local_t;

typedef struct symtable_record_global {
    char *key;
    symtable_global_data_t data;
} symtable_record_global_t;

// ***************************************** SYMTABLES ******************************************
// defined as arrays of pointers to records (with set size)
typedef symtable_record_local_t* local_symtable[ST_SIZE];
typedef symtable_record_global_t* global_symtable[ST_SIZE];





// ***************************************** FUNCTIONS ******************************************
// All functions returning int return ERR_CODE_ST_OK on success, otherwise ERR_CODE_ST_FAIL
// Functions accept table in form of void pointer, because of the different types of tables
// Type of table needs to be specified in parameter table_type_t type

int symtable_init(void *table, table_type_t type);
void symtable_free(void *table);
int symtable_insert(void *table, char *key, void *data, table_type_t type);
void *symtable_search(void *table, char *key, symbol_type_t type, table_type_t table_type);
// int symtable_insert(symtable_t *table, char *key, symtable_data_t data);
// symtable_record_t *symtable_search(symtable_t *table, char *key, symbol_type_t type, scope_type_t scope);
// int symtable_remove(symtable_t *table, char *key);

// void for_each(symtable_t *table, void (*callback)(symtable_record_t *record)); 
// bool index_occupied(symtable_t *table, int index);
// int next_free_index(symtable_t *table, int starting_index);


// void symtable_free(symtable_t *table);

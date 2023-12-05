#pragma once

#include <stdio.h>
#include <stdlib.h>


/**
 * @Macro for the initial capacity of the dynamic array
*/

#define INITIAL_CAPACITY 16



/**
 * @brief Structure for the dynamic array item
 * @param data - pointer to the data, its type is void* because it can be any type, it is up to the user to cast it to the desired type
 * @typedef ArrayItem
*/
typedef struct ArrayItem
{
    void *data;
} ArrayItem;


/**
 * @brief Structure for the dynamic array
 * @param items - pointer to the array of items
 * @param size - the current size of the array
 * @param capacity - the current capacity of the array
 * @typedef DynamicArray
*/
typedef struct DynamicArray
{
    ArrayItem *items;
    unsigned size;
    unsigned capacity;
} DynamicArray;


/**
 * @brief Function to initialize the dynamic array
 * @param array - pointer to the dynamic array
 * @note The function expects a valid pointer to the dynamic array, so it can modify it
*/
void arrayInit(DynamicArray *array);



/**
 * @brief Inserts an item to the dynamic array
 * @note The function creates the ArrayItem structure for the data
 * @param array - pointer to the dynamic array
 * @param data - pointer to the data to be inserted
*/
void arrayInsert(DynamicArray *array, void *data);


/**
 * @brief Removes an item from the dynamic array
 * @param array - pointer to the dynamic array
 * @param index - the index of the item to be removed
*/

void arrayRemove(DynamicArray *array, unsigned index);


/**
 * @brief Disposes the dynamic array
 * @param array - pointer to the dynamic array
*/

void arrayDispose(DynamicArray *array);


/**
 * @brief Returns the size of the dynamic array
 * @param array - pointer to the dynamic array
 * @return The size of the dynamic array
*/

unsigned arraySize(DynamicArray *array);


/**
 * @brief Reverses the dynamic array
 * @param array - pointer to the dynamic array to be reversed
*/

void arrayReverse(DynamicArray *array);

/**
 * @brief Returns the item at the given index
 * @param array - pointer to the dynamic array
 * @param index - the index of the item to be returned
 * @return The item at the given index
 * @note The function returns a pointer to the item
*/
ArrayItem *arrayGet(DynamicArray *array, unsigned index);




#pragma once

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 16

typedef struct ArrayItem
{
    void *data;
} ArrayItem;

typedef struct DynamicArray
{
    ArrayItem *items;
    unsigned size;
    unsigned capacity;
} DynamicArray;

void arrayInit(DynamicArray *array);

void arrayInsert(DynamicArray *array, void *data);

void arrayRemove(DynamicArray *array, unsigned index);

void arrayDispose(DynamicArray *array);

unsigned arraySize(DynamicArray *array);

void arrayReverse(DynamicArray *array);

ArrayItem *arrayGet(DynamicArray *array, unsigned index);




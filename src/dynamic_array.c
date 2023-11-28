#include "header_files/dynamic_array.h"

void arrayInit(DynamicArray *array)
{
    if (array == NULL)
    {
        return;
    }
    array->capacity = INITIAL_CAPACITY;
    array->size = 0;
    array->items = malloc(sizeof(ArrayItem *) * array->capacity);
}

void arrayInsert(DynamicArray *array, void *data)
{
    if (array == NULL)
    {
        return;
    }

    if (array->size == array->capacity)
    {
        array->capacity *= 2;
        array->items = realloc(array->items, sizeof(ArrayItem *) * array->size);
    }

    array->items[array->size].data = data;
    array->size++;
}

void arrayRemove(DynamicArray *array, unsigned index)
{

    if (array == NULL)
    {
        return;
    }
    if (array->items == NULL)
    {
        return;
    }

    if (index >= array->size)
    {
        return;
    }

    for (unsigned i = index; i < array->size - 1; i++)
    {
        array->items[i].data = array->items[i + 1].data;
    }

    array->size--;
}

void arrayDispose(DynamicArray *array)
{
    if (array == NULL)
    {
        return;
    }
    if (array->items == NULL)
    {
        return;
    }

    free(array->items);
    array->items = NULL;
    array->size = 0;
    array->capacity = 0;
}

unsigned arraySize(DynamicArray *array)
{
    if (array == NULL)
    {
        return 0;
    }
    return array->size;
}

void arrayReverse(DynamicArray *array)
{
    if (array == NULL)
    {
        return;
    }
    if (array->items == NULL)
    {
        return;
    }

    for (unsigned i = 0; i < array->size / 2; i++)
    {
        void *tmp = array->items[i].data;
        array->items[i].data = array->items[array->size - i - 1].data;
        array->items[array->size - i - 1].data = tmp;
    }

    if (array->size % 2 == 1 && array->size > 2)
    {
        void *tmp = array->items[array->size / 2].data;
        array->items[array->size / 2].data = array->items[array->size / 2 + 2].data;
        array->items[array->size / 2 + 2].data = tmp;
    }
}

ArrayItem *arrayGet(DynamicArray *array, unsigned index) {
    if (array == NULL)
    {
        return NULL;
    }
    if (array->items == NULL)
    {
        return NULL;
    }

    if (index >= array->size)
    {
        return NULL;
    }

    return &array->items[index];
}
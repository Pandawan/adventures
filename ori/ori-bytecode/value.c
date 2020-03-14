#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value)
{
    // If the "code" byte array is too small, grow it
    if (array->capacity < array->count + 1)
    {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(array->values, Value, oldCapacity, array->capacity);
    }

    // Add the new element
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array)
{
    // Deallocate all of the memory
    FREE_ARRAY(ValueArray, array->values, array->capacity);
    // Re-initialize the value array to a blank state
    initValueArray(array);
}

void printValue(Value value)
{
    printf("%g", value);
}

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
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
    switch (value.type)
    {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NULL:
            printf("null");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_OBJ:
            printObject(value);
            break;
    }
}

bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type)
        return false;

    switch (a.type)
    {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NULL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:
        {
            // TODO: Values equal for obj but non-string
            ObjString* aString = AS_STRING(a);
            ObjString* bString = AS_STRING(b);
            return aString->length == bString->length &&
                   memcmp(aString->chars, bString->chars, aString->length) == 0;
        }
    }
}

#ifndef ori_value_h
#define ori_value_h

#include "common.h"

// TODO: Maybe add some macros for "generic" dynamic arrays because this is duplicate of Chunk

// A constant/literal value
typedef double Value;

// A dynamic array of all the values (in a particular chunk)
typedef struct
{
    int capacity;
    int count;
    Value* values;
} ValueArray;

// Initialize a new ValueArray
void initValueArray(ValueArray* array);
// Append a value at the end of the given array
void writeValueArray(ValueArray* array, Value value);
// Free the given value array
void freeValueArray(ValueArray* array);
// Prints the given value in a nicely formatted way
void printValue(Value value);

#endif

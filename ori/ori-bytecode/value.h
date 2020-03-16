#ifndef ori_value_h
#define ori_value_h

#include "common.h"

// Types a value can represent
typedef enum
{
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
} ValueType;

typedef struct
{
    ValueType type;
    // Union allows for multiple fields to overlap in memory
    // This is useful in that we don't need to create extra memory allocation
    // if we only ever use one of the fields at a time
    union {
        bool boolean;
        double number;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

// Converts the given ori bool to a native C bool
#define AS_BOOL(value) ((value).as.boolean)
// Converts the given ori number ot a native C double
#define AS_NUMBER(value) ((value).as.number)
// NOTE: No need for AS_NULL because there's only ONE null value

// Convert the given native C bool to ori bool
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
// Convert the given native C null to ori null
#define NULL_VAL ((Value){VAL_NULL, {.number = 0}})
// Convert the given native c double to a ori number
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// TODO: Maybe add some macros for "generic" dynamic arrays because this is duplicate of Chunk

// A dynamic array of all the values (in a particular chunk)
typedef struct
{
    int capacity;
    int count;
    Value* values;
} ValueArray;

// Whether or not two given values are equal
bool valuesEqual(Value a, Value b);
// Initialize a new ValueArray
void initValueArray(ValueArray* array);
// Append a value at the end of the given array
void writeValueArray(ValueArray* array, Value value);
// Free the given value array
void freeValueArray(ValueArray* array);
// Prints the given value in a nicely formatted way
void printValue(Value value);

#endif

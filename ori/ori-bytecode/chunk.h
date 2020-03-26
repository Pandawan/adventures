#ifndef ori_chunk_h
#define ori_chunk_h

#include "common.h"
#include "value.h"

// Operations
typedef enum
{
    // Load a constant from the constant array
    // Operand(s): Index in the constant array
    // TODO: Implement a OP_CONSTANT_LONG to allow for more than 256 different constants per chunk (See challenges for C14)
    OP_CONSTANT,
    // Store null, true, and false as operations rather than constants in a table
    OP_NULL,
    OP_TRUE,
    OP_FALSE,

    OP_POP,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,

    // Binary operations
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    // TODO: For better performance, implement an OP_NOT_EQUAL, OP_GREATER_EQUAL, OP_LESS_EQUAL rather than turn it into two operations
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    // Unary operations
    OP_NOT,
    OP_NEGATE,

    OP_PRINT,
    OP_RETURN,
} OpCode;

typedef struct
{
    int count;
    int capacity;
    // Dynamic array of instructions & data
    uint8_t* code;
    // Array of line numbers to correspond operation (from code) to a specific line
    // This is especially useful for reporting runtime errors,
    // where the location of the offending code/operation is.
    // TODO: Implement compression for repeated lines (See Challenges for C14)
    int* lines;
    // Dynamic array of all constants/literals
    ValueArray constants;
} Chunk;

// Initialize a new chunk
void initChunk(Chunk* chunk);
// Free the chunk from memory
void freeChunk(Chunk* chunk);
// Append a byte to the end of the chunk
// (reporting the line where that byte appears in source code)
void writeChunk(Chunk* chunk, uint8_t byte, int line);
// Add a constant/literal to the chunk
// Returns the index of the newly added constant
int addConstant(Chunk* chunk, Value value);

#endif

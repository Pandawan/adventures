#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk)
{
    // Deallocate all of the memory
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(unsigned int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    // Re-initialize the chunk to a blank state
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, unsigned int line)
{
    // If the "code" byte array is too small, grow it
    if (chunk->capacity < chunk->count + 1)
    {
        unsigned int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(chunk->lines, unsigned int, oldCapacity, chunk->capacity);
    }

    // Add the new element
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int addConstant(Chunk* chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

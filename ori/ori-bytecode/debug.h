#ifndef ori_debug_h
#define ori_debug_h

#include "chunk.h"

// Disassemble all of the instructions in the chunk
void disassembleChunk(Chunk*, const char* name);
// Disassembles a single instruction at a given offset
// Returns the offset for the next instruction
int disassembleInstruction(Chunk* chunk, int offset);

#endif

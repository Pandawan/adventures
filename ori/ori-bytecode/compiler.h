#ifndef ori_compiler_h
#define ori_compiler_h

#include "vm.h"

// Compile the given source code as bytecode into the given chunk
// Returns true if it succeeded
bool compile(const char* source, Chunk* chunk);

#endif

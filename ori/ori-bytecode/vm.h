#ifndef ori_vm_h
#define ori_vm_h

#include "chunk.h"
#include "value.h"

// TODO: Should there be a dynamic stack (still with a max amount but bigger than this)
#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    // Instruction Pointer
    // Pointer to the location of the instruction that is currently being executed
    uint8_t* ip;
    // Stack of current values (0 is bottom)
    Value stack[STACK_MAX];
    // Points ONE element ahead of the last value
    Value* stackTop;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
// Interpret the given source code
InterpretResult interpret(const char* source);
// Push the given value to the top of the stack
void push(Value value);
// Pop the top value off the stack
Value pop();

#endif
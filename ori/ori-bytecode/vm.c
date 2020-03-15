#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

// For now, use a single global vm instance
// TODO: Might want to pass around a pointer to VM everywhere to allow for embedding, etc.
VM vm;

static void resetStack()
{
    // No need to clear the stack for the VM as it is a constant array for the entire lifetime of the VM
    // We can just overwrite it whenever we need to reset it
    vm.stackTop = vm.stack;
}

void initVM()
{
    resetStack();
}

void freeVM()
{
}

static InterpretResult run()
{
// Reads the next byte from the bytecode (and advances the instruction pointer)
#define READ_BYTE() (*vm.ip++)
// Reads the next byte, treats it as constant index, find the value at that index
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// Run a binary operation with the given operator on the next two values
// NOTE: Using a do while in order to wrap this boilerplate in a code block
// so it doesn't conflict with other code when processed
// Also, since value is a stack, b comes out first
#define BINARY_OP(op)     \
    do                    \
    {                     \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

    // Infinite loop until result
    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        // Print all values in the stack
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        // If debugging, print the disassembled instruction (need pointer math to get the offset after it has been advanced by READ_BYTE())
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        // Loop through instructions one at a time to process it
        switch (instruction = READ_BYTE())
        {
            // TODO: If wanted to optimize "decoding" (aka "dispatching") of instructions, look up “direct threaded code”, “jump table”, and “computed goto”
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                // Push the newly read constant onto the stack
                push(constant);
                printValue(constant);
                printf("\n");
                break;
            }
            // TODO: Maybe add bitwise operators
            case OP_ADD:
                BINARY_OP(+);
                break;
            case OP_SUBTRACT:
                BINARY_OP(-);
                break;
            case OP_MULTIPLY:
                BINARY_OP(*);
                break;
            case OP_DIVIDE:
                BINARY_OP(/);
                break;
            case OP_NEGATE:
                push(-pop());
                break;
            case OP_RETURN:
            {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

// Clean up macros that are only needed here
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
    compile(source);
    return INTERPRET_OK;
}

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    // No need to remove the value, it'll eventually be overwritten anyway
    return *vm.stackTop;
}
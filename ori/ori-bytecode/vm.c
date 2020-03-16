#include <stdarg.h>
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

static void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);

    // TODO: Perhaps do some kind of recovery from runtime errors
    // TODO: Stack trace

    resetStack();
}

void initVM()
{
    resetStack();
}

void freeVM()
{
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

// Get the value a specific distance down from the top of the stack without popping it
// 0 is top of the stack
static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

// TODO: Should this be part of value.c? (with header)
static bool isFalsy(Value value)
{
    // null and false are the only falsy values
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
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
// This also checks for type
#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
                                                        \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
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

            case OP_NULL:
                push(NULL_VAL);
                break;
            case OP_TRUE:
                push(BOOL_VAL(true));
                break;
            case OP_FALSE:
                push(BOOL_VAL(false));
                break;

            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;
            // TODO: Maybe add bitwise operators
            case OP_ADD:
                BINARY_OP(NUMBER_VAL, +);
                break;
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;

            case OP_NOT:
                push(BOOL_VAL(isFalsy(pop())));
                break;

            case OP_NEGATE:
            {
                if (!IS_NUMBER(peek(0)))
                {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }
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
    Chunk chunk;
    initChunk(&chunk);

    // Fill the chunk with compiled bytecode
    if (!compile(source, &chunk))
    {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void repl()
{
    // TODO: Allow for infinite line length and multiline input
    char line[1024];

    // Start infinite repl loop
    for (;;)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // Get required buffer size by going to the end of the file and counting how many bytes we've passed
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // TODO: Support UTF-8 rather than just char/ASCII
    char* buffer = (char*)malloc(fileSize + 1);
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    // Check that it read the file correctly
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    // Terminate string with null byte
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path)
{
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    // readFile gives ownership of the string, so need to free it once done
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int main(int argc, const char* argv[])
{
    initVM();

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        runFile(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: ori [path]\n");
        exit(64);
    }

    freeVM();

    return 0;
}

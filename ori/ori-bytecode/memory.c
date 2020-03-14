#include <stdlib.h>

#include "common.h"
#include "memory.h"

void* reallocate(void* previous, size_t oldSize, size_t newSize)
{
    // Handles freeing data
    if (newSize == 0)
    {
        free(previous);
        return NULL;
    }

    // Already handles new allocation and resizing
    return realloc(previous, newSize);
}

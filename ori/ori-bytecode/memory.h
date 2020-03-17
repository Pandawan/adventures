#ifndef ori_memory_h
#define ori_memory_h

#include "object.h"

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

// Frees the given pointer of the given type (reallocating it to 0)
#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

// Calculates a new capacity based on current given capacity
// Initially use size 8, then double it if more is needed
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity)*2)

// Grows the array to the new size for the given type
#define GROW_ARRAY(previous, type, oldCount, count)        \
    (type*)reallocate(previous, sizeof(type) * (oldCount), \
                      sizeof(type) * (count))

// Frees the given array, by reallocating with new size 0
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

// Dynamic memory management, used for allocating, resizing, freeing, etc.
// Returns void* which is a pointer to data of "any type"
void* reallocate(void* previous, size_t oldSize, size_t newSize);
// Frees all dynamically allocated objects on the VM's linked list
void freeObjects();

#endif

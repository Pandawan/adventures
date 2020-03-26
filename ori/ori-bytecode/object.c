#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

// Equivalent to a "base class constructor" to initialize the Obj state
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    // Insert newly allocated object to the VM's list
    object->next = vm.objects;
    vm.objects = object;

    return object;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    // Add the string to the VM's string interning table
    // NOTE: See table.c's findEntry TODO note. 
    // This could be done only in a special Symbols object if wanted.
    tableSet(&vm.strings, string, NULL_VAL);

    return string;
}

static uint32_t hashString(const char* key, int length) {
    // FNV-1a hash algorithm
    uint32_t hash = 2166136261u;
    for(int i = 0; i < length; i++) {
        hash ^= key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    // Check if this exact string already exits, if so, instead of copying it, just pass it
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        // Also need to free up the chars passed because no longer need it
        // And takeString takes ownership of it so it has to handle freeing it
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    // Check if this exact string already exists, if so, instead of copying it, just pass it
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

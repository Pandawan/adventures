#ifndef ori_object_h
#define ori_object_h

#include "common.h"
#include "value.h"

// Extracts object type tag from a given value
#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum
{
    OBJ_STRING
} ObjType;

struct sObj
{
    ObjType type;
    // Reference to the next Obj (making a linked list)
    // This is used for the GC to have a list of all dynamically allocated Objs
    struct sObj* next;
};

struct sObjString
{
    // First field is obj so that a pointer can easily
    // be converted between one and the other
    Obj obj;
    int length;
    // TODO: Use "flexible array members" to avoid having an extra pointer to chars
    // TODO: Allow for unicode UTF-8 rather than ASCII char
    char* chars;
    // Cached hash of the string so it doesn't have to be calculated multiple times
    uint32_t hash;
};

// Convert the given c-string into an ObjString (taking ownership of the c-string)
ObjString* takeString(char* chars, int length);
// Convert the given c-string into an ObjString (copying the characters)
ObjString* copyString(const char* chars, int length);

// Print the given object value
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif

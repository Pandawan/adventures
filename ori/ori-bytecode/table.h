#ifndef ori_table_h
#define ori_table_h

#include "common.h"
#include "value.h"

// Grow whenever the table's array is at least 75% full
// NOTE: 75% is a totally arbitrary number, in reality needs to be carefully benchmarked and picked
#define TABLE_MAX_LOAD 0.75

typedef struct {
    // Since key is always a string, store it as ObjString pointer rather than converting it
    // TODO: Maybe want to add support for number keys? (essentially reduce it to a sequence of bits rather than ObjString* specifically)
    // This would allow for arrays to just be HashTables with number keys
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
// Get the value in the table at the given key and store it in the passed value parameter
// Returns true if the key exists
bool tableGet(Table* table, ObjString* key, Value* value);
// Adds the given key/value pair to the given hash table
// Returns true if a new entry was added (instead of replacing an existing one)
bool tableSet(Table* table, ObjString* key, Value value);
// Removes the entry with the given key from the table
// Returns true if it succesfully removed it
bool tableDelete(Table* table, ObjString* key);
// Add all of the entries from the from table into the to table
void tableAddAll(Table* from, Table* to);
// Find a given table entry with the given c-string key and hash
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif

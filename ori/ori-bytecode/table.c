#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

void initTable(Table* table)
{
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

// Find a "position/bucket" for the given key
// This can be used both for finding position to place a new entry in
// Or to find a pre-existing entry by key
static Entry* findEntry(Entry* entries, int capacity, ObjString* key)
{
    // Initial index where first expect the entry to be
    uint32_t index = key->hash % capacity;

    // Keep track of any tombstones found that could be used
    Entry* tombstone = NULL;

    // Loop until it finds an empty bucket OR it finds one with the same key
    // This will never be infinite because the entries array grows every time it's "close to being full"
    for (;;)
    {
        Entry* entry = &entries[index];

        if (entry->key == NULL)
        {
            if (IS_NULL(entry->value))
            {
                // Empty entry
                // If found a tombstone on the way there, then might as well replace it
                // Otherwise, use the newly found empty entry
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // Found a tombstone
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        // TODO: This uses string interning for ALL strings, which means a collection of all strings is kept track of by runtime
        // This has pros and cons, some languages have a "symbol" type for interning and use "strings" for non-intern
        // If you didn't use string interning, you'd compare hash codes of the two strings, and if they're the same you'd then compare characters
        else if (entry->key == key)
        {
            // Found the key
            return entry;
        }

        // If that was the wrong key, linearly search for them (next one)
        index = (index + 1) % capacity;
    }
}

bool tableGet(Table* table, ObjString* key, Value* value)
{
    if (table->count == 0)
        return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(Table* table, int capacity)
{
    Entry* entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }

    // Since not copying over tombstones, the count might be different
    table->count = 0;
    // Since just resized the array, need to re-calculate all of the entries' positions
    // Because the position depends on hash % capacity and the capacity has just changed
    for (int i = 0; i < table->capacity; i++)
    {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL)
            continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // Free the old array, just moved everythong over
    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    // Increase count only if created a new key
    // (not from a tombstone since tombstones are already counted into count)
    if (isNewKey && IS_NULL(entry->value))
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key)
{
    if (table->count == 0)
        return false;

    // Find the entry
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL)
        return false;

    // Place a tombstone in the entry
    // This is done to not break the collision chain
    entry->key = NULL;
    entry->value = BOOL_VAL(true);

    return true;
}

void tableAddAll(Table* from, Table* to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL)
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash)
{
    if (table->count == 0)
        return NULL;

    uint32_t index = hash % table->capacity;

    for (;;)
    {
        Entry* entry = &table->entries[index];

        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry
            if (IS_NULL(entry->value))
                return NULL;
        }
        else if (entry->key->length == length && entry->key->hash == hash && mcmcmp(entry->key->chars, length) == 0)
        {
            // Found it
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

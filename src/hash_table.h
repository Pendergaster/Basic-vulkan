/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

// Motivation for this: General purpose hash table to just use something easily and prehaps remove later

// Collision stragedy: Linearly probing
// Resize stragedy: Pick next pre computed prime number which is about 2 times larger than last size
// Remove stragedy: Find value and re insert values after it
// Hash: djb2 (needs more studying which would be good in general case)

#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include "defs.h"


// Pre-calulcated prime numbers, if larger than these hope is already lost
static const u32 MAP_PRIMES[] = {
    53,
    97,
    193,
    389,
    769,
    1543,
    3079,
    6151,
    12289,
    24593,
    49157,
    98317,
    196613,
    393241,
    786433,
    1572869,
    3145739,
    6291469,
    12582917,
    25165843,
    50331653,
    100663319,
    201326611,
    402653189,
    805306457,
    1610612741
};

static int
hashmap_find_primeindex(u32 mustSize) {
    int div = SIZEOF_ARRAY(MAP_PRIMES);
    int currentIndex = 0;
    while(1) {
        div = div / 2;
        if(div == 0) break;
        if(mustSize > MAP_PRIMES[currentIndex]) {
            currentIndex += div;
        } else if (mustSize < MAP_PRIMES[currentIndex]) {
            currentIndex -= div;
        } else {
            break;
        }
    }

    if(mustSize >= MAP_PRIMES[currentIndex]) {
        return currentIndex + 1;
    }

    return currentIndex;
}


#define DECLARE_HASHTABLEKEY(TYPE, PRETYPE)\
    typedef struct PRETYPE##Key {\
        TYPE    key;\
        u32    hash;\
    } PRETYPE##Key;\
    typedef TYPE PRETYPE##KeyType;

#define DECLARE_HASHTABLE(KEYTYPE, PRETYPE, PREFUNC)\
    typedef struct PRETYPE##HashTable {\
        u32                 size;\
        KEYTYPE             *keys;\
        u8*                 values;\
        u32                 sizeOfValue;\
        u32                 primeIndex;\
        u32                 numInserted;\
    } PRETYPE##HashTable;\
    \
    /* mutation of djb2 hash*/\
    static inline u32 PREFUNC##_hashtable_hash(u8* key, u32 size) {\
        u32 hash = 5381;\
        i32 c;\
        while (size--) {\
            c = *key++;\
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */\
        }\
        /* Ensure that 0 cant ever be picked so it can be value of available slot*/\
        return hash ? hash : 1;\
    }\
    \
    static void PREFUNC##_hashtable_init(PRETYPE##HashTable* table, u32 sizeOfValue, u32 primeIndex) {\
        \
        table->primeIndex = primeIndex;\
        table->sizeOfValue = sizeOfValue;\
        table->size = MAP_PRIMES[primeIndex];\
        table->keys = (KEYTYPE*)malloc(sizeof *table->keys * table->size + sizeOfValue * table->size);\
        memset(table->keys, 0, sizeof *table->keys * table->size);\
        table->values = (u8*)table->keys + sizeof *table->keys * table->size;\
        table->numInserted = 0;\
    }\
    \
    static void PREFUNC##_hashtable_dispose(PRETYPE##HashTable* table) {\
        free(table->keys);\
        *table = (PRETYPE##HashTable) {0};\
    }\
    \
    static inline void _##PREFUNC##_hashtable_insert(PRETYPE##HashTable* table,const KEYTYPE* key, u8* value) {\
        \
        u32 insertLocation = key->hash % table->size;\
        /* Search for available slot*/\
        while(table->keys[insertLocation].hash != 0) {\
            insertLocation += 1;\
        }\
        table->keys[insertLocation].key = key->key;\
        table->keys[insertLocation].hash = key->hash;\
        /* Copy value to index*/\
        memcpy(table->values + (insertLocation * table->sizeOfValue), value, table->sizeOfValue);\
        \
    }\
    \
    static void PREFUNC##_hashtable_resize(PRETYPE##HashTable* table) {\
        \
        table->primeIndex += 1;\
        u32 oldSize = table->size;\
        KEYTYPE* oldKeys = table->keys;\
        u8* oldValues = table->values;\
        \
        table->size = MAP_PRIMES[table->primeIndex];\
        table->keys = (KEYTYPE*)malloc(sizeof *table->keys * table->size + table->sizeOfValue * table->size);\
        memset(table->keys, 0, sizeof *table->keys * table->size);\
        table->values = (u8*)table->keys + sizeof *table->keys * table->size;\
        \
        /* Re insert all keys and values*/\
        for(u32 i = 0; i < oldSize; i++) {\
            /* check if hash is not 0*/\
            if(oldKeys[i].hash != 0 ) {\
                _##PREFUNC##_hashtable_insert(table, &oldKeys[i], oldValues + ( i * table->sizeOfValue));\
            }\
        }\
        \
        free(oldKeys);\
    }\
    \
    static void PREFUNC##_hashtable_insert(PRETYPE##HashTable* table, KEYTYPE##Type key, u8* value) {\
        \
        /* Resize if table is 80% full to reduce collisions*/\
        if((float)(table->numInserted + 1) / (float)table->size  > 0.8) {\
            PREFUNC##_hashtable_resize(table);\
        }\
        \
        table->numInserted += 1;\
        \
        KEYTYPE _key = {\
            .key = key,\
            .hash = PREFUNC##_hashtable_hash((u8*)&key, table->sizeOfValue)\
        };\
        \
        _##PREFUNC##_hashtable_insert(table,&_key,value );\
    }\
    \
    static void* PREFUNC##_hashtable_access(PRETYPE##HashTable* table, KEYTYPE##Type key) {\
        \
        u32 hash = PREFUNC##_hashtable_hash((u8*)&key, table->sizeOfValue);\
        u32 location = hash % table->size;\
        \
        while(table->keys[location].hash != 0) {\
            if(table->keys[location].hash == hash && memcmp(&table->keys[location].key, &key, sizeof key) == 0) break;\
            \
            location = (location + 1) % table->size;\
        }\
        \
        if(table->keys[location].hash == 0) return NULL;\
        \
        return (table->values + ( location * table->sizeOfValue));\
    }\
    \
    static void PREFUNC##_hashtable_remove(PRETYPE##HashTable* table, KEYTYPE##Type key) {\
        u8* value = (u8*)PREFUNC##_hashtable_access(table, key);\
        /* Nothing to remove*/\
        if(value == NULL) return;\
        u32 valuesIndex = (value - table->values) / table->sizeOfValue;\
        \
        /* Remove wanted key*/\
        table->keys[valuesIndex] = (KEYTYPE) {0};\
        \
        /* Reinsert values after the removed value*/\
        valuesIndex = (valuesIndex + 1) % table->size;\
        while(table->keys[valuesIndex].hash) {\
            /* Save old one and zero it indicating that its free*/\
            KEYTYPE _key = table->keys[valuesIndex];\
            table->keys[valuesIndex] = (KEYTYPE) {0};\
            \
            _##PREFUNC##_hashtable_insert(table, &_key, table->values + ( valuesIndex * table->sizeOfValue));\
            \
            valuesIndex = (valuesIndex + 1) % table->size;\
        }\
        \
        table->numInserted -= 1;\
    }\
    \

#endif /* HASH_TABLE_H */


//TODO hash function out of declaration
//TODO project style naming

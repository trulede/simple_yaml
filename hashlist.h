/*
Copyright (c) 2021 Timothy Rule
MIT License
*/

#ifndef HASHLIST_H
#define HASHLIST_H


#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <hashmap.h>


#define HASHLIST_KEY_LEN        (10+1)

typedef struct HashList {
    HashMap     hash;
} HashList;


static __inline__ int hashlist_init(HashList *h) {
    return hashmap_init(&h->hash);
}

static __inline__ void hashlist_destroy(HashList *h) {
    assert(h);
    hashmap_destroy(&h->hash);
}

static __inline__ uint32_t hashlist_length(HashList *h) {
    assert(h);
    return h->hash.used_nodes;
}

static __inline__ void hashlist_append(HashList *h, void *value) {
    assert(h);
    char key[HASHLIST_KEY_LEN];
    snprintf(key, HASHLIST_KEY_LEN, "%i", hashlist_length(h));
    hashmap_set(&h->hash, key, value);
}

static __inline__ void* hashlist_get_at(HashList *h, uint32_t index) {
    assert(h);
    char key[HASHLIST_KEY_LEN];
    snprintf(key, HASHLIST_KEY_LEN, "%i", index);
    return hashmap_get(&h->hash, key);
}

#endif /* HASHLIST_H */

#pragma once
#include <stdbool.h>
#include <stdint.h>

struct cache_obj;
typedef struct cache_obj *cache_t;

typedef const uint8_t *key_type;
typedef const void *val_type;
typedef uint64_t (*hash_func)(key_type key);

cache_t create_cache(uint64_t maxmem,hash_func h_fn);
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size);
// Add a <key, value> pair to the cache.
// If key already exists, it will overwrite the old value.
// If maxmem capacity is exceeded, sufficient values will be removed
// from the cache to accomodate the new value.
void cache_set(cache_t cache, key_type key, val_type val, uint32_t val_size);

// Delete an object from the cache, if it's still there
void cache_delete(cache_t cache, key_type key);

// Compute the total amount of memory used up by all cache values (not keys)
uint64_t cache_space_used(cache_t cache);

// Destroy all resource connected to a cache object
void destroy_cache(cache_t cache);

#define create_cache_wrapper(maxmem,hash) create_cache(maxmem,hash)
#define cache_get_wrapper(cache,key,val_size) cache_get(cache,key,val_size)
/*
inline cache_t create_cache_wrapper(uint64_t maxmem, hash_func hash){
    return create_cache(maxmem,hash);
}

inline void *cache_get_wrapper(cache_t cache,uint8_t *key, uint32_t *val_size){
    return cache_get(cache,key,val_size);
}
*/

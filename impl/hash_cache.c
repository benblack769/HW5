#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cache.h"
#include "helper.h"
#include "replacement.h"

struct key_val_obj{
    key_type key;
    val_type val;
    uint32_t val_size;

    pinfo_t policy_info;
};
typedef struct key_val_obj key_val_s;
struct link_obj;
typedef struct link_obj* link_t;
struct link_obj{
    key_val_s data;
    link_t next;
};
void del_link(cache_t cache, link_t * obj);

uint64_t def_hash_fn(key_type key);

size_t map_to_location(uint64_t hash_val,size_t table_size){
    return hash_val % table_size;
}

struct cache_obj{
    uint64_t maxmem;
    uint64_t mem_used;
    link_t * table;
    size_t table_size;
    size_t num_elements;
    hash_func h_fn;
    policy_t evic_policy;
};
const size_t default_table_size = 2503;//medium size prime number

struct user_identifier{//declared in replacement.h
    link_t * linkpp;
};

cache_t create_cache(uint64_t maxmem,hash_func h_fn){
    cache_t n_cache = calloc(1,sizeof(struct cache_obj));

    n_cache->maxmem = maxmem;
    n_cache->mem_used = 0;
    n_cache->table_size = default_table_size;
    n_cache->num_elements = 0;
    n_cache->table = calloc(default_table_size,sizeof(key_val_s));
    n_cache->h_fn = (h_fn == NULL) ? def_hash_fn : h_fn;
    n_cache->evic_policy = create_policy(maxmem);

    return n_cache;
}

link_t * querry_hash(cache_t cache, key_type key){
    //returns the pointer to the pointer of the key, if the key exists,
    // and the pointer to the location the key would be, if it were added, if it is not there
    size_t hash_loc = map_to_location(cache->h_fn(key),cache->table_size);
    link_t * cur_item = &cache->table[hash_loc];
    while(*cur_item != NULL && strcmp((char*)(key),(char*)((*cur_item)->data.key))){
        cur_item = &(*cur_item)->next;
    }
    return cur_item;
}
void assign_to_link(link_t * linkp,key_val_s data){
    if(*linkp == NULL){
        *linkp = calloc(1,sizeof(struct link_obj));
    }
    (*linkp)->data = data;
}
void resize_table(cache_t cache,uint64_t new_size){
    const size_t old_t_size = cache->table_size;
    link_t * old_table = cache->table;

    cache->table = calloc(new_size,sizeof(link_t));
    cache->table_size = new_size;

    //add the pointers into the new cache table without changing the data at all
    for(size_t i = 0; i < old_t_size;i++){
        link_t cur_l = old_table[i];
        while(cur_l != NULL){
            link_t next_l = cur_l->next;

            cur_l->next = NULL;
            *querry_hash(cache,cur_l->data.key) = cur_l;

            cur_l = next_l;
        }
    }
    free(old_table);
}
bool should_add_evict_deletions(cache_t cache,uint32_t val_size){
    struct id_arr add_res = ids_to_delete_if_added(cache->evic_policy,val_size);
    if(add_res.should_add){
        for(size_t di = 0;di < add_res.size; di++){
            // the data given to the policy is the pointer to the key of that the cache is storing,
            //so use that to find the data and delete it
            cache_delete(cache,(key_type)(add_res.data[di]));
        }
    }
    //the array of ids needs to be freed
    if(add_res.data != NULL){
        free(add_res.data);
    }
    return add_res.should_add;
}
void add_to_cache(cache_t cache, key_type key, val_type val, uint32_t val_size){
    //adds in a new item in the location of the cache
    key_type key_copy = make_copy(key,strlen((char*)key)+1);

    key_val_s new_item;
    new_item.key = key_copy;
    new_item.val = make_copy(val,val_size);
    new_item.val_size = val_size;
    //give the policy the pointer to the key so that it can read the value of the key when
    //the policy teels it about evictions in ids_to_delete_if_added
    new_item.policy_info = create_info(cache->evic_policy,(void*)(key_copy),val_size);

    assign_to_link(querry_hash(cache,key),new_item);
    cache->mem_used += val_size;
    cache->num_elements++;

    //resizes table if load factor is over 0.5
    if(cache->num_elements*2 > cache->table_size){
        resize_table(cache,cache->table_size*2);
    }
}

void cache_set(cache_t cache, key_type key, val_type val, uint32_t val_size){
    link_t * init_link = querry_hash(cache,key);
    //if the item is already in the list, then delete it
    del_link(cache,init_link);
    //if the policy tells the cache not to add the item, do not add it
    if(!should_add_evict_deletions(cache,val_size)){
        return;
    }
    add_to_cache(cache,key,val,val_size);
}
val_type cache_get(cache_t cache, key_type key, uint32_t *val_size){
    link_t hash_l = *querry_hash(cache,key);
    if(hash_l != NULL){
        //if the item is in the cache, tell the policy that fact, and return the value
        info_gotten(cache->evic_policy,hash_l->data.policy_info);
        *val_size = hash_l->data.val_size;
        return hash_l->data.val;
    }
    else{
        //if the item is not in the cache, return NULL
        *val_size = 0;
        return NULL;
    }
}
void del_link(cache_t cache,link_t * obj){
    //deletes the thing pointed to by the obj and replaces it with the next thing in the linked list  (a NULL if it is at the end)
    link_t myobj = *obj;
    if(*obj != NULL){
        *obj = myobj->next;

        cache->mem_used -= myobj->data.val_size;
        cache->num_elements--;

        free((uint8_t*)myobj->data.key);
        free((void *)myobj->data.val);

        delete_info(cache->evic_policy,myobj->data.policy_info);

        free(myobj);
    }
}
void cache_delete(cache_t cache, key_type key){
    del_link(cache,querry_hash(cache,key));
}
uint64_t cache_space_used(cache_t cache){
    return cache->mem_used;
}
void destroy_cache(cache_t cache){
    //deletes the links
    for(size_t i = 0; i < cache->table_size; i++){
        while(cache->table[i] != NULL){
            del_link(cache,&cache->table[i]);
        }
    }
    delete_policy(cache->evic_policy);
    free(cache->table);
    free(cache);
}

uint64_t def_hash_fn(key_type key){
    //xors the bits of the string together
    size_t tot_size = strlen((char*)(key));
    const size_t out_size = sizeof(uint64_t);
    uint64_t out = 0;
    for(size_t i = 0; i < tot_size / out_size;i++){
        out ^= ((uint64_t *)(key))[i];
    }
    for(size_t i = 0;i < tot_size%out_size; i++){
        size_t index = i + tot_size - tot_size%out_size;
        out ^= ((uint64_t)(key[index])) << (i*BITS_IN_BYTE);
    }
    return out;
}

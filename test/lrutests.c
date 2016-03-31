#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lrutests.h"
#include "test_helper.h"

/* There's something really fishy going on here:
When we only input 10 elements we get
Akosik: pass
Jcosel: pass
zzhong: fail

on inputting 11 elements we get:
Akosik: pass
jcosel: fail
zzhong: crash

on inputting 20 elements we get:
akosik: pass
jcosel: crash
zzhong: crash

alec's breaks on element 36. works up until 35. looool
*/
bool evictions_occur(){
    //adds a ton of elements to a cache with a small maxmem and sees if any elements are evicted
    const uint64_t max_elmts = 10;
    const uint64_t num_elmts_add = 36;
    printf("hihihi\n");
    fflush(stdout);
    cache_t cache = create_cache_wrapper(max_elmts*sizeof(int_ty),NULL);
    printf("hihihi11111\n");
    fflush(stdout);
    add_elements(cache,0,num_elmts_add,INT);
    printf("hihihi222222\n");
    fflush(stdout);
    bool evicted = elements_dont_exist(cache,0,num_elmts_add-max_elmts);
    printf("hihihi33333\n");
    fflush(stdout);
    bool not_evicted = elements_exist(cache,num_elmts_add-max_elmts,num_elmts_add);
    printf("hihihi44444\n");
    fflush(stdout);
    destroy_cache(cache);
    return evicted && not_evicted;
}

bool basic_lru_test(){
    //adds A then B then gets A then adds C and expects B to be evicted
    const size_t max_mem = 2*sizeof(int_ty)+1;
    cache_t cache = create_cache_wrapper(max_mem,NULL);
    add_element(cache,0,INT);
    add_element(cache,1,INT);
    //gets the element
    element_exists(cache,0);
    add_element(cache,2,INT);
    bool worked = !element_exists(cache,1);
    destroy_cache(cache);
    return worked;
}

bool lru_delete_test(){
    //adds A then B then C then gets A then deletes B adds D and expects C to be evicted
    const size_t max_mem = 2*sizeof(int_ty)+1;
    cache_t cache = create_cache_wrapper(max_mem,NULL);
    add_element(cache,0,INT);
    add_element(cache,1,INT);
    //gets the element
    element_exists(cache,0);
    add_element(cache,2,INT);
    bool worked = !element_exists(cache,1);
    destroy_cache(cache);
    return worked;
}

bool update_reordering(){
    // Adds A and B, then updates A, and adds a large val C, expecting
    // that B will be evicted and A will be held.
        // need to make this val big enough so we don't run into problems
        // with ambiguity regarding whether we evict >maxmem or ≥ maxmem.
    cache_t c = create_cache_wrapper(4*sizeof(int),NULL);
    key_type k1 = "key1";
    int v1 = 1;
    key_type k2 = "key2";
    int v2 = 2;
    cache_set(c,k1,&v1,sizeof(int));
    cache_set(c,k2,&v2,sizeof(int));
    int v3 = 3;
    // this should update the val stored under k1 and now
    // we expect (k2,v2) to be the LRU element.
    printf("updating val\n");
    fflush(stdout);
    cache_set(c,k1,&v3,sizeof(int));

    key_type k3 = "key3";
    val_type v4 = "largeval";
    printf("largeval\n");
    fflush(stdout);
    //we expect this to evict (k2,v2)
    cache_set(c,k3,v4,strlen(v4)+1);

    int size1,size2;
    printf("hihihihihihihi\n");
    fflush(stdout);
    val_type out1 = cache_get_wrapper(c,k1,&size1);
    if (out1 == NULL ) return false;
    val_type out2 = cache_get_wrapper(c,k2,&size2);
    if (out2 != NULL) return false;
        // need to place ifs on sepearate lines there is an issue
        // with all cache_gets sharing the same pointer
    printf("hihihihihihihi2222222222\n");
    fflush(stdout);
    destroy_cache(c);
    return true;

}

bool evict_on_reset_old_val(){
    // Adds A and B, then updates B with a value that would overload
    // the cache if it was added but not if it replaced B. We expect B to be
    // replaced and A to not be evicted.
    cache_t c = create_cache_wrapper(3*sizeof(int),NULL);
    key_type k1 = "key1";
    int v1 = 1;
    key_type k2 = "key2";
    int v2 = 2;
    cache_set(c,k1,&v1,sizeof(int));
    cache_set(c,k2,&v2,sizeof(int));

    // This should reassign the value under k2, but if the
    // LRU doesn't check the size of what an element would be
    // after reassigning then we would also end up evicting k1
    val_type v3 = "five";
    cache_set(c,k2,v3,strlen(v3)+1);

    // This should be null
    int size;
    val_type out = cache_get_wrapper(c,k1,&size);

    if(out==NULL){
        destroy_cache(c);
        return false;
    }
    destroy_cache(c);
    return true;
}

bool evict_on_failed_reset_old_val(){
    // Adds A and B, then updates B with a value that is larger than the entire
    // maxmem. We expect both A and B to not be evicted, but this is ambiguous in
    // the spec, so we will make more mention of this later in the writeup
    cache_t c = create_cache_wrapper(3*sizeof(int),NULL);
    key_type k1 = "key1";
    int v1 = 1;
    key_type k2 = "key2";
    int v2 = 2;
    cache_set(c,k1,&v1,sizeof(int));
    cache_set(c,k2,&v2,sizeof(int));

    // This should fail to reassign the value of key2, since the new
    // value is too large for the cache. We want to make sure that
    // none of the old values were evicted.
    val_type v3 = "value too large for cache!";
    cache_set(c,k2,v3,strlen(v3)+1);

    // This should not be null
    int size1,size2;
    val_type out1 = cache_get_wrapper(c,k1,&size1);
    if(out1==NULL){
        destroy_cache(c);
        return false;
    }
    printf("We will pass!1\n");
    val_type out2 = cache_get_wrapper(c,k2,&size1);
    if(out2==NULL){
        destroy_cache(c);
        return false;
    }
    printf("We will pass!2\n");
    if(*(int*)out2!=v2){
        destroy_cache(c);
        return false;
    }
    printf("We will pass!3\n");
    destroy_cache(c);
    return true;
}

bool get_reordering(){
    // Adds A then B, the gets A (and expects LRU to be reordered). Adds C,
    // which causes one element to be evicted, and expects that B (not A)
    // was evicted.
    cache_t c = create_cache_wrapper(3*sizeof(int),NULL);
    key_type k1 = "key1";
    int v1 = 1;
    key_type k2 = "key2";
    int v2 = 2;
    cache_set(c,k1,&v1,sizeof(int));
    cache_set(c,k2,&v2,sizeof(int));

    // This should bump k1 to the front of the LRU line
    // ie. now the LRU element should be k2
    int size;
    val_type out1 = cache_get_wrapper(c,k1,&size);

    // This should cause us to evict k2
    key_type k3 = "val_too_big";
    val_type v3 = "five";
    cache_set(c,k3,v3,strlen(v3)+1);

    // This should be null
    val_type out2 = cache_get_wrapper(c,k2,&size);

    destroy_cache(c);
    return out2 == NULL;
}

bool maxmem_not_excceeded(){
    //adds too many elements, checks if the size of values with non-null keys is > maxmem, deletes some, adds more, overwrites some, checks again
    const uint64_t max_emts = 100;
    const uint64_t max_mem = max_emts*sizeof(int_ty);
    cache_t cache = create_cache_wrapper(max_mem,NULL);

    add_elements(cache,0,max_emts+1,INT);
    bool exceeded = space_of_elements(cache,0,max_emts+1,INT) > max_mem;
    delete_elements(cache,0,max_emts+1);
    add_elements(cache,max_emts*2,max_emts*3,INT);
    exceeded = exceeded || space_of_elements(cache,max_emts*2,max_emts*3,INT) > max_mem;

    destroy_cache(cache);
    return !exceeded;
}
bool elements_not_evicted_early(){
    //adds some elements, deletes some, and replaces some, and then checks if all the elements are still in the cache
    const uint64_t max_emts = 100;
    const uint64_t max_add_emts = max_emts-1;//due to ambiguity about whether the cache can store maxmem or up to, but not including maxmem
    cache_t cache = create_cache_wrapper(max_emts*sizeof(int_ty),NULL);

    add_elements(cache,0,max_add_emts/2,INT);
    delete_elements(cache,0,max_add_emts/4);
    add_elements(cache,max_add_emts/2, max_add_emts/4 + max_add_emts,INT);
    bool passed = elements_exist(cache,max_add_emts/4,max_add_emts/4 + max_add_emts);
    destroy_cache(cache);
    return passed;
}
bool var_len_evictions(){
    //basic lru_test for variable length strings
    const size_t max_mem = val_size(0,STR)+val_size(1,STR)+val_size(2,STR)-1;
    cache_t cache = create_cache_wrapper(max_mem,NULL);
    add_element(cache,0,STR);
    add_element(cache,1,STR);
    //gets the element
    element_exists(cache,0);
    add_element(cache,2,STR);
    bool worked = !element_exists(cache,1);
    destroy_cache(cache);
    return worked;
}

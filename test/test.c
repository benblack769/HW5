#include <string.h>
#include "test.h"
#include "test_helper.h"

bool cache_space_preserved(){
    // adds, deletes, updates, and maybe evicts and sees if the total size of
    // items of items in the cache is the size of all of the non-NULL elements
    const uint64_t maxmem = 200;
    cache_t c = create_cache_wrapper(maxmem,NULL);
    add_elements(c,0,5,STR);
    delete_element(c,4);
    add_elements(c,0,2,INT);
    bool worked = cache_space_used(c) == space_of_elements(c,0,2,INT) + space_of_elements(c,2,5,STR);
    destroy_cache(c);
    return worked;
}

// we have a weird error with jcosel where it doubles its maxmem cap if
// we exceed maxmem. i dont know what kind of test i'd use to expose it though

bool create_init_correct_mem(){
    // cache space used should still be 0 because we shouldnt be
    // able to add an element greater than the size of the cache
    // fails jcosel because of resize on caches too small
    // fails zzhong because doesn't check if new val exceeds maxmem
    cache_t c = create_cache_wrapper(65,NULL); //set over 64 for jcosel
    key_type k = "key";
    val_type v = "string too long! string too long! string too long! \
    string too long! string too long! string too long! string too long!";
    cache_set(c,k,v,strlen(v)+1);
    int space = cache_space_used(c);
    destroy_cache(c);
    if (space!=0) return false;
    return true;
}

bool add_single_item_over_memmax(){
    //adds a single item over maxmem and sees if it is not in the cache.
    cache_t c = create_cache_wrapper(65,NULL); //set over 64 for jcosel
    key_type k = "key";
    val_type v = "string too long! string too long! string too long! \
    string too long! string too long! string too long! string too long!";
    cache_set(c,k,v,strlen(v)+1);
    uint32_t null_size = 0;
    val_type cache_big_val = cache_get_wrapper(c,k,&null_size);
    destroy_cache(c);
    return cache_big_val == NULL;
}


bool large_val_copied_correctly(){
    // Tests cache_set on an array containing two large values. If vals
    // were treated as strings, this would fail.
    cache_t cache = create_cache_wrapper(1000,NULL);
    key_type key = "normal key";
    uint64_t val[] = {0xff00ff00ff00ffff,0xcc00cc00fe00ddcc};
    cache_set(cache,key,&val,sizeof(uint64_t)*2);
    uint32_t size = -1;
    val_type outval = cache_get_wrapper(cache,key,&size);
    bool worked;
    if(outval != NULL){
        uint64_t * out_arr = (uint64_t *)(outval);
        worked = out_arr[0] == val[0] && out_arr[1] == val[1];
    }
    else{
        worked = false;
    }
    destroy_cache(cache);
    return worked;
}

bool get_huge_value(){
    // adds vals under different keys that start with the same character.
    // if the cache doesn't copy keys by string then this will fail.
    const size_t valsize = 50000;
    cache_t c = create_cache_wrapper(valsize,NULL);
    char * str = calloc(valsize,1);
    char * key = "mykey";
    for(int i = 0; i < valsize-5; i++){
        str[i] = rand()%26+96;//random lower case letters
    }
    cache_set(c,key,str,strlen(str)+1);
    uint32_t num = 0;
    void * val = cache_get(c,key,&num);
    bool worked = val != NULL && strcmp(val,str) == 0;
    free(str);
    destroy_cache(c);
    return worked;
}


bool add_over_memmax_eviction(){
    // jcosel gets a false pass because they resize their maxmem
    // adds small items to cache and then adds an item larger than maxmem
    // and sees if items have been evicted. (expect them to not be).
    uint64_t max_mem = 10;
    char rand_key[] = "random_key";
    char big_val[] = "string of length > max_mem asdfasdfasdfasdfasdfasdfasdf";

    cache_t c = create_cache_wrapper(max_mem*sizeof(uint16_t),NULL);
    add_elements(c,0,3,INT);
    cache_set(c,rand_key,big_val,strlen(big_val)+1);

    if (elements_dont_exist(c,0,3)) return false;
    // int size;
    // if (cache_get_wrapper(c,rand_key,&size) != NULL) return false;
    destroy_cache(c);
    return true;
}

bool add_resize_buckets_or_maxmem(){
    // adds small items up to half of maxmem then attemps to add
    // an item that would be too large for the cache (unless maxmem changed
    // after the resize). If new item appears, then maxmem was changed
    // in resize (which is a bug - maxmem should be constant).
    uint64_t max_mem = 10;
    char rand_key[] = "random_key";
    char big_val[] = "string of length > max_mem";

    cache_t c = create_cache_wrapper(max_mem*sizeof(uint16_t),NULL);
    add_elements(c,0,5,INT);

    int space1 = cache_space_used(c);

    cache_set(c,rand_key,big_val,strlen(big_val)+1);
    uint32_t null_size = 0;
    val_type cache_big_val = cache_get_wrapper(c,rand_key,&null_size);

    int space2 = cache_space_used(c);

    if (cache_big_val != NULL){
        printf("%d\n",space1);
        printf("%d\n",space2);
        printf("%s\n",cache_big_val);
        printf("%p\n",cache_big_val);
        fflush(stdout);
        return false;
    }
    destroy_cache(c);
    return cache_big_val == NULL;
}

bool get_null_empty(){
    // adds things to our cache and then attempts to get one that
    // doesn't exist
    uint64_t max_mem = 100;
    cache_t c = create_cache_wrapper(max_mem*sizeof(int_ty)+1,NULL);
    add_elements(c,0,max_mem,INT);
    char * k = "key";
    int size1;
    void * out = cache_get_wrapper(c,k,&size1);
    destroy_cache(c);
    if (out != NULL) return false;
    return true;
}

bool get_nonexist(){
    // attempts to get an elements that doesn't exist in an empty cache
    cache_t c = create_cache_wrapper(1000,NULL);
    key_type k = "nonexist";
    int size;
    val_type out = cache_get_wrapper(c,k,&size);
    destroy_cache(c);
    if(out != NULL) return false;
    return true;
}


bool get_size_after_reassign_test(){
    // Tests if space from cache_get remains the same after reassigning a val
    cache_t c = create_cache_wrapper(1000,NULL);
    char * k = "key";
    int v1 = 10;
    int size1,size2;
    cache_set(c,k,&v1,(sizeof(int)));
    void * out = cache_get_wrapper(c,k,&size1);

    char *v2 = "stringval";
    cache_set(c,k,v2,strlen(v2)+1);
    out = cache_get_wrapper(c,k,&size2);
    destroy_cache(c);
    if(size1 == size2){
        printf("%s\n",out);
        printf("%d\n",size1);
        printf("%d\n",size2);
        return false;
    }
    return true;
}

// exposes some problems with the val that cache_get returns.
// if we don't copy out the value to something totally new,
// we end up reassigning the same pointer over and over.
// so updating it raises an error because we also update
// old outs.
bool get_val_after_reassign_test(){
    // Tests if the val from cache_get remains the same after reassigning a val
    cache_t c = create_cache_wrapper(1000,NULL);
    char * k = "key";
    char *v1 = "stringval1";
    int size1,size2;
    cache_set(c,k,v1,strlen(v1)+1);
    void * out1 = cache_get_wrapper(c,k,&size1);
    printf("%s\n",out1);
    printf("%p\n",out1);
    char *v2 = "stringval2";
    cache_set(c,k,v2,strlen(v2)+1);
    printf("%s\n",out1);
    printf("%p\n",out1);
    void * out2 = cache_get_wrapper(c,k,&size2);
    printf("11%s\n",out1);
    printf("%p\n",out2);
    if(strcmp(out1,out2)==0 ||
        strcmp(out1,v1)!=0 ||
        strcmp(out2,v2)!=0){
        printf("%s\n",out1);
        printf("%s\n",out2);
        printf("%s\n",v1);
        printf("%s\n",v2);
        destroy_cache(c);
        return false;
    }
    destroy_cache(c);
    return true;
}


bool get_with_null_term_strs_test(){
    // Tests keys cache_set on two different keys that contain a null termination in
    // the middle: "a\0b" and "a\0c". We expect cache_set to overwrite the first val
    // with the second val because both keys 'look the same' (ie "a\0").
    cache_t cache = create_cache_wrapper(100,NULL);
    key_type key1 = "a\0b";
    key_type key2 = "a\0c";
    uint64_t val1 = 10;
    uint64_t val2 = 11;
    cache_set(cache,key1,&val1,sizeof(val1));
    cache_set(cache,key2,&val2,sizeof(val2));
    uint32_t size = -1;
    val_type outval = cache_get_wrapper(cache,key1,&size);
    bool worked = (*(uint64_t * )(outval) == val2);
    destroy_cache(cache);
    return worked;
}

bool delete_not_in(){
    // Tests to see if something that is set and then deleted returns NULL
    // when cache_get is called.
    cache_t cache = create_cache_wrapper(max_str_len+1,NULL);
    const uint64_t item = 10;
    add_element(cache,item,STR);
    delete_element(cache,item);
    bool worked = !element_exists(cache,item);

    destroy_cache(cache);
    return worked;
}

bool delete_affect_get_out(){
    // A bug was raised with the outputed vals of cache_get being affected
    // by updates. This tests whether we have the same problem on the outputs
    // of cache_get after deletes.
    cache_t c = create_cache_wrapper(1000,NULL);
    char * k = "this is my cool key";
    char *v1 = "stringval1";
    int size1,size2;
    cache_set(c,k,v1,strlen(v1)+1);
    void * out1 = cache_get_wrapper(c,k,&size1);
    cache_delete(c,k);
    void * out2 = cache_get_wrapper(c,k,&size1);
    printf("%s\n",out1);
    printf("%p\n",out2);
    // printf("%s\n",out1);
    // printf("%p\n",out1);
    destroy_cache(c);
    if (out1 == NULL) return false;
    return true;
}

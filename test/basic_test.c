#include <stdlib.h>

#include "basic_test.h"
#include "test_helper.h"
// Naive create_cache test - makes sure we don't crash
// and that we don't end up with a NULL pointer when
// creating a cache.
bool create_test(){
    cache_t c = create_cache_wrapper(1000,NULL);
    if (c==NULL){
    	return false;
    }
    return true;
}

//Naive destroy_cache test - make sure we aren't crashing
// when we destroy our cache
bool destroy_test(){
	cache_t c = create_cache_wrapper(1000,NULL);
	destroy_cache(c);
	return true;
}

// Naive cache_get test
bool add_test(){
    // Adds many items of differnet sizes (some of which with identical keys),
    // all of which below maxmem. Returns true if it does not crash
    const uint64_t num_adds = 20;
    cache_t c = create_cache_wrapper(num_adds*max_str_len,NULL);
    add_elements(c,0,num_adds,INT);
    add_elements(c,0,num_adds/2,STR);
    destroy_cache(c);
    return true;
}

bool crash_on_memoverload(){
    // if fail to return true, we crashed on overloading
    // crashes aledger because of assert on val too large. could be
    // called a bug if we count these crashes (ie. not handling
    // these cases) as bugs.
    cache_t c = create_cache_wrapper(65,NULL); //set over 64 for jcosel
    key_type k = "key";
    val_type v = "string too long! string too long! string too long! \
    string too long! string too long! string too long! string too long!";
    cache_set(c,k,v,strlen(v)+1);
    destroy_cache(c);
    return true;
}

// Naive cache_get test - tests to see if we correctly update size
bool get_size_test(){
	cache_t c = create_cache_wrapper(1000,NULL);
	key_type k= "key";
	int v = 12345;
	cache_set(c,k,&v,(sizeof(int)));
	int size;
	void * out = cache_get_wrapper(c,k,&size);
    destroy_cache(c);
    if (size != sizeof(int)){
        // printf("wrong size\n");
        // need this or compiler does smth strange on -O2
        return false;
    }
    return true;
}

// Naive cache_get test - tests to see if we return correct val
bool get_val_test(){
	cache_t c = create_cache_wrapper(1000,NULL);
	key_type k= "key";
	int v = 12345;
	cache_set(c,k,&v,(sizeof(int)));
	int size;
	void * out = cache_get_wrapper(c,k,&size);
    bool worked = v == *(int*)out;
    destroy_cache(c);
    return worked;
}

// Naive cache_delete test - makes sure we don't crash
// when trying to delete.
bool delete_test(){
	cache_t c = create_cache_wrapper(1000,NULL);
	key_type k= "key";
	int v = 12345;
	cache_set(c,k,&v,(sizeof(int)));
	cache_delete(c,k);
	destroy_cache(c);
	return true;
}

// Naive cache_space_used test - if the space of things added
//(everything well below maxmem) is what cache_space_used returns
bool space_test(){
	cache_t c = create_cache_wrapper(10000,NULL);
    add_elements(c,0,100,INT);
    int size = cache_space_used(c);
    destroy_cache(c);
    return (size==100*(sizeof(int_ty)));
}

bool hash_called = false;
uint64_t custom_hash(key_type key){
    hash_called = true;
    return 0;//constant function is a valid hash
}

bool custom_hash_is_called(){
    //checks if the custom hash function specified is called on add, get, update, and delete
    const uint64_t item = 5;
    cache_t cache =  create_cache_wrapper(1000,custom_hash);

    hash_called = false;
    add_element(cache,item,INT);
    bool add_hash = hash_called;

    hash_called = false;
    element_exists(cache,item);
    bool get_hash = hash_called;

    hash_called = false;
    add_element(cache,item,STR);
    bool update_hash = hash_called;

    hash_called = false;
    delete_element(cache,item);
    bool delete_hash = hash_called;

    destroy_cache(cache);
    return add_hash && get_hash && update_hash && delete_hash;
}

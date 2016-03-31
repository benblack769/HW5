#include <stdbool.h>
// adds a ton of elements to a cache with a small maxmem and sees if any elements are evicted
bool evictions_occur();

// adds A then B then gets A then adds C and expects B to be evicted
bool basic_lru_test();

// adds A then B then C then gets A then deletes B adds D and expects C to be evicted
bool lru_delete_test();

// tests whether updating elements properly reorders the LRU
bool update_reordering();

// tests whether we do unnecessary evictions when updating an existing val
// this one is tricky because the "expected behavior" isn't clearly defined
// in our spec. this tests for a specific behavior (ie not evicting unecessarily)
// but we'll also make a note of this in the writeup!
bool evict_on_failed_reset_old_val();

// makes sure that cache_get properly reorders the LRU
bool get_reordering();

// adds too many elements, checks if the size of values with non-null keys is
// > maxmem, deletes some, adds more, overwrites some, checks again
bool maxmem_not_excceeded();

bool evict_on_reset_old_val();

// adds some elements, deletes some, and replaces some, and then checks
// if all the elements are still in the cache
bool elements_not_evicted_early();

// basic lru_test for variable length strings
bool var_len_evictions();




/*
need to rewrite new tests according to our testing module
l . o . l.
*/

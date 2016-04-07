#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define failed_code 0x92
typedef bool (*test_fn_ty)();


bool create_init_correct_mem();


bool add_single_item_over_memmax();
bool large_val_copied_correctly();
bool get_huge_value();
bool add_over_memmax_eviction();
bool add_resize_buckets_or_maxmem();


bool get_null_empty();
bool get_nonexist();
bool get_size_after_reassign_test();
bool get_val_after_reassign_test();
bool get_with_null_term_strs_test();


bool delete_not_in();
bool delete_affect_get_out();
bool cache_space_preserved();


// Below : Unimplemented tests that may or may not be doable.

// bool delete_nonelement_test();
//bool delete_post_resize_test(); impossible?
// bool delete_empty_reuse_test(); //can the cache be reused after being emptied?

// bool space_str_test(); //(others are actually redundant here) //for int/str/bool/array
// bool space_evict_test();
//bool space_resize_test(); impossible
// bool space_delete_test();
// bool space_clear_test();//what is this?

//more advanced tests to get crashes out of the more secure caches
// bool random_set_get_delete();//my own test
//time dependent tests (need a timing framework)
// bool reasonable_access_time_few_items();
// bool reasonable_access_time_many_items();
// bool long_str_collision_test();

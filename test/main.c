#include <string.h>
#include <inttypes.h>
#include "cachewrapper.h"
#include "test_helper.h"
#include "test.h"
#include "basic_test.h"
#include "lrutests.h"


#define RunTest(testname) {\
    printf(#testname "\n");\
    fflush(stdout);\
    bool worked = testname();\
    free_vals();\
    if(worked)\
        exit(0);\
    else\
        exit(failed_code);\
}

int main(int argn,char ** argv){
#ifndef NDEBUG
    uint64_t test_num = 20;
#else
    if(argn != 2){
        printf("needs one argument\n");
        exit(1);
	}
	char * num_str = argv[1];
    uint64_t test_num = strtoumax(num_str,NULL,10);

#endif
    gen_vals();

	switch (test_num) {
	// 0 - 8: naive and basic crash tests
	case 0:
		RunTest(create_test);
		break;
	case 1:
		RunTest(destroy_test);
		break;
	case 2:
		RunTest(add_test);
		break;
	case 3:
		RunTest(crash_on_memoverload);
   		break;
	case 4:
		RunTest(get_size_test);
		break;
	case 5:
		RunTest(get_val_test);
		break;
	case 6:
		RunTest(delete_test);
		break;
	case 7:
		RunTest(space_test);
		break;
	case 8:
		RunTest(custom_hash_is_called);
		break;
    // 9 more advanced cache_space_used test
	case 9:
		RunTest(cache_space_preserved);
		break;
   	// 10 - 14: cache_set test
	case 10:
		RunTest(add_single_item_over_memmax);
		break;
	case 11:
		RunTest(large_val_copied_correctly); // jhepworth (crash)
		break;
   	case 12:
   		RunTest(add_same_starting_char);
   		break;
    case 13:
    	RunTest(add_over_memmax_eviction); // only pass or crash
    	break;
    case 14:
    	RunTest(add_resize_buckets_or_maxmem);
    	break;
	// 15 - 19: cache_get tests:
   	case 15:
   		RunTest(get_null_empty) //exposes no bugs
   		break;
   	case 16:
   		RunTest(get_nonexist); // exposes no bugs
   		break;
 	case 17:
		RunTest(get_size_after_reassign_test);
        break;
    case 18:
    	RunTest(get_val_after_reassign_test);
    	break;
	case 19:
		RunTest(get_with_null_term_strs_test); //jhepworth (crash)
		break;

    // 20 - 21: cache_delete test:
    case 20:
    	RunTest(delete_not_in); //jhepworth (crash)
    	break;
    case 21:
    	RunTest(delete_affect_get_out); // exposes no bugs
    	break;
    // 22 - 31: LRU tests
	case 22:
		RunTest(evictions_occur);
		break;
	case 23:
		RunTest(basic_lru_test);
		break;
	case 24:
		RunTest(lru_delete_test);
		break;
   	case 25:
   		RunTest(update_reordering); //infinite loop on aledger
   		break;
   	case 26:
   		RunTest(evict_on_reset_old_val);
   		break;
   	case 27:
   		RunTest(evict_on_failed_reset_old_val); //infinite loop on aledger
   		break;
   	case 28:
   		RunTest(get_reordering);
   		break;
	case 29:
		RunTest(maxmem_not_excceeded); //only pass or crash
		break;
	case 30:
		RunTest(elements_not_evicted_early); //only pass or crash
		break;
	case 31:
		RunTest(var_len_evictions);
		break;
	default:
		printf("test not implemented\n");
		return -1;
		break;

	}

	printf("\n");
    return 0;
}

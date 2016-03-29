#include <stdbool.h>
// Naive tests - ie make sure we don't crash and that basic
// functionality is there.
bool create_test();
bool destroy_test();
bool add_test();
bool crash_on_memoverload();
bool get_size_test();
bool get_val_test();
bool delete_test();
bool space_test();
bool custom_hash_is_called();//needs a global variable to work

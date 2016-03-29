#include <stdbool.h>
#include "cachewrapper.h"
/*
The core concept behind this file is this:

Instead of manually keeping track of keys and values of different and comparing
them, there are simply keys and values associated with integers, so you can do
operations on ranges of integers.

You can also specify the type of the value, STR or INT. STR translates to a
string of upper case letters of total length (including null byte) max_str_len.
INT is a 16 bit signed integer.

Keys are currently implemented as the integer value of the index you pass in.
It is careful about the int being null terminated.

One thing to look out for is that these indexes are only valid if they are less
than num_ivals for INT and num_svals for STR
*/

#define num_ivals 1000000
#define num_svals 200000
#define max_str_len 100
typedef int16_t int_ty;
typedef enum {INT,STR} val_entry_type;

void gen_vals();
void free_vals();

cache_t create_no_overflow(uint64_t maxelmt);
void add_element(cache_t cache,uint64_t elnum,val_entry_type ent_ty);
void add_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty);
bool check_element(cache_t cache,uint64_t elmt,val_entry_type ent_ty);
bool check_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty);
void delete_element(cache_t cache,uint64_t elmt);
void delete_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt);
uint32_t space_of_element(cache_t cache,uint64_t elmt,val_entry_type ent_ty);
uint64_t space_of_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty);
uint32_t reported_space_of_element(cache_t cache,uint64_t elmt);
uint64_t reported_space_of_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt);
bool element_exists(cache_t cache,uint64_t elmt);
bool elements_exist(cache_t cache,uint64_t start_elmt,uint64_t end_elmt);
bool elements_dont_exist(cache_t cache,uint64_t start_elmt,uint64_t end_elmt);

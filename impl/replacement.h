#pragma once
#include <stdbool.h>
#include "helper.h"
//abstract structs that the replacement policy defines
struct policy_obj;
struct pinfo_obj;
typedef struct policy_obj* policy_t;
typedef struct pinfo_obj* pinfo_t;

//abstract data that the user (i.e. the cache) uses to understand the output of ids_to_delete_if_added
//defined to be a void * so that it can really be any data, including a struct, integer, or pointer to any type, the replacement policy does not care
typedef void* user_id_t;

policy_t create_policy(uint64_t maxmem);
void delete_policy(policy_t policy);

struct id_arr{
    user_id_t * data;
    size_t size;
    bool should_add;
};
//adds the info to the policy (if "id" is already in there, then it duplicates the value)
//the policy owns the pointer, do not free!!! (use delete_info)
//user owns the id pointer
pinfo_t create_info(policy_t policy,user_id_t id, uint32_t val_size);
//removes info from the policy
void delete_info(policy_t policy,pinfo_t info);

//if the should_add value is zero (false), then, the value should not be added to the cache at all (for instance, val_size > max_mem)
//otherwise, it returns an array of all the cache_ids that need to be deleted for the value to be added
//the user owns the pointer, free the array before it goes out of scope!
struct id_arr ids_to_delete_if_added(policy_t policy, uint32_t val_size);

//gives policy information about the cache.
void info_gotten(policy_t policy,pinfo_t info);

#include <assert.h>
#include "replacement.h"

struct policy_obj{
    uint64_t maxmem;
    uint64_t used_mem;
    //a bidirectional linked list with the most recently used at top, least recently used at bottom
    pinfo_t top;
    pinfo_t bottom;
};

struct pinfo_obj{
    //the link the the bidirectional linked list
    pinfo_t low;
    pinfo_t high;
    user_id_t ident;
    uint32_t val_size;
};
typedef struct pinfo_obj pinfo_s;

bool is_empty(policy_t policy){
    return policy->bottom == NULL;
}

policy_t create_policy(uint64_t maxmem){
    policy_t newobj = calloc(1,sizeof(struct policy_obj));

    newobj->maxmem = maxmem;
    newobj->top = NULL;
    newobj->bottom = NULL;

    return newobj;
}

void delete_policy(policy_t policy){
    //traverses the entire linked list, deleting elements one by one
    pinfo_t obj = policy->bottom;
    while(obj != NULL){
        pinfo_t next = obj->high;
        free(obj);
        obj = next;
    }
    free(policy);
}
void move_info_to_head(policy_t policy,pinfo_t info){
    pinfo_t old_head = policy->top;
    if(old_head != NULL){
        old_head->high = info;
    }

    info->low = old_head;
    info->high = NULL;

    policy->top = info;
    if(policy->bottom == NULL){
        policy->bottom = info;
    }
}
void remove_info_from_list(policy_t policy,pinfo_t info){
    //removes from list by changing list data
    //does not change the info at all

    //connects the links on either side to each other instead of info
    if(info->high != NULL){
        info->high->low = info->low;
    }
    if(info->low != NULL){
        info->low->high = info->high;
    }
    //alters policy if you are deleting the head or the tail
    if(policy->top == info){
        policy->top = info->low;
    }
    if(policy->bottom == info){
        policy->bottom = info->high;
    }
}

pinfo_t create_info(policy_t policy, user_id_t id, uint32_t val_size){
    assert(!(policy->maxmem < policy->used_mem + val_size) && "policy is asked to hold more memory than it should");
    //create new object
    pinfo_t newinf = calloc(1,sizeof(pinfo_s));
    newinf->val_size = val_size;
    newinf->ident = id;

    move_info_to_head(policy,newinf);

    policy->used_mem += val_size;

    return newinf;
}

void delete_info(policy_t policy,pinfo_t info){
    remove_info_from_list(policy,info);

    policy->used_mem -= info->val_size;

    free(info);
}
size_t num_to_delete(policy_t policy, uint32_t val_size,pinfo_t * out_t){
    size_t num_del_ids = 0;
    pinfo_t cur_t = policy->bottom;
    uint64_t tot_mem = policy->used_mem + val_size;
    while(tot_mem > policy->maxmem && cur_t != NULL){
        tot_mem -= cur_t->val_size;
        cur_t = cur_t->high;
        num_del_ids++;
    }
    *out_t = cur_t;
    return num_del_ids;
}
user_id_t * make_item_array(size_t num_items,pinfo_t bottom){
    user_id_t * arr = calloc(num_items,sizeof(user_id_t));
    //unsafe traversal of items in the list, placing in arr
    pinfo_t cur_i = bottom;
    for(size_t di = 0; di < num_items; di++){
        arr[di] = cur_i->ident;
        cur_i = cur_i->high;
    }
    return arr;
}

struct id_arr ids_to_delete_if_added(policy_t policy, uint32_t val_size){
    pinfo_t final_item = NULL;
    size_t num_del_ids = num_to_delete(policy,val_size,&final_item);

    //if the val_size is greater than the maxmem, don't bother adding the item
    if(val_size > policy->maxmem){
        struct id_arr retval = {NULL,0,false};
        return retval;
    }
    //if it doesn't need to delete anything, then return that
    else if(num_del_ids == 0){
        struct id_arr retval = {NULL,0,true};
        return retval;
    }
    user_id_t * id_arr = make_item_array(num_del_ids,policy->bottom);

    struct id_arr retval = {id_arr,num_del_ids,true};
    return retval;//caller frees arr
}

void info_gotten(policy_t policy,pinfo_t info){
    remove_info_from_list(policy,info);

    move_info_to_head(policy,info);
}

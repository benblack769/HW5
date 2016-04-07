#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "test_helper.h"

int_ty * ivals;
char ** svals;

#define maxintlen 22
char keyarr[maxintlen] = {0};

void load_key_to_intarr(uint64_t num){
    for(size_t i = 0; i < maxintlen; i++){
        keyarr[i] = 0;
    }
	sprintf(keyarr, PRIu64, num);
}
char rand_char(){
	return rand()%26+64;//random upper case letters
}
char * gen_rand_str(){
	size_t str_size = rand() % (max_str_len - 1) + 1;
	char * newstr = calloc(str_size,sizeof(char));
	for(size_t cn = 0; cn < str_size-1; ++cn){
		newstr[cn] = rand_char();
	}
	newstr[str_size-1] = 0;
	return newstr;
}
void gen_cvals(){
	svals = malloc(num_svals*sizeof(*svals));
	for(size_t sn = 0; sn < num_svals; sn++){
		svals[sn] = gen_rand_str();
	}
}
void gen_ivals(){
	ivals = malloc(num_ivals*sizeof(int_ty));
	for(size_t i = 0; i < num_ivals; i++){
		ivals[i] = rand_char();
	}
}
void gen_vals(){
	gen_ivals();
	gen_cvals();
}
void free_vals(){
	free(ivals);
	for(size_t sn = 0; sn < num_svals; sn++){
		free(svals[sn]);
	}
	free(svals);
}
void * val_ptr(size_t loc, val_entry_type ent_ty){
	return ent_ty == INT ? &ivals[loc] : (ent_ty == STR ? svals[loc] : NULL);
}
uint32_t val_size(size_t loc, val_entry_type ent_ty){
	return ent_ty == INT ? sizeof(int_ty) : (ent_ty == STR ? strlen(svals[loc])+1 : 0);
}
uint64_t to_key_int(uint64_t num){
	return num & 0x00ffffffffffffff;
}
bool check_val(val_type val,size_t loc,val_entry_type ent_ty){
	return (val != NULL) &&
		   ((ent_ty == INT) ?
				*(int_ty *)(val) == ivals[loc] :
				(ent_ty == STR ?
					strcmp(svals[loc],val) == 0 :
					false));
}
cache_t create_no_overflow(uint64_t maxelmt){
	return create_cache_wrapper(maxelmt*max_str_len,NULL);
}
void add_element(cache_t cache,uint64_t elnum,val_entry_type ent_ty){
	load_key_to_intarr(elnum);
	cache_set(cache,keyarr,val_ptr(elnum,ent_ty),val_size(elnum,ent_ty));
}
void add_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty){
	for(size_t i = start_elmt; i < end_elmt; ++i){
		add_element(cache,i,ent_ty);
	}
}
bool check_element(cache_t cache,uint64_t elmt,val_entry_type ent_ty){
	load_key_to_intarr(elmt);
	uint32_t null_size = 0;
	val_type val = cache_get_wrapper(cache,keyarr,&null_size);
	return check_val(val,elmt,ent_ty);
}
bool check_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty){
	bool res = true;
	for(size_t i = start_elmt; i < end_elmt; ++i){
		res = res && check_element(cache,i,ent_ty);
	}
	return res;
}
void delete_element(cache_t cache,uint64_t elmt){
    load_key_to_intarr(elmt);
	cache_delete(cache,keyarr);
}
void delete_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt){
	for(uint64_t i = start_elmt; i < end_elmt; ++i){
		delete_element(cache,i);
	}
}
uint32_t space_of_element(cache_t cache,uint64_t elmt,val_entry_type ent_ty){
	load_key_to_intarr(elmt);
	uint32_t null_size = 0;
	val_type null_val = cache_get_wrapper(cache,keyarr,&null_size);
	return null_val == NULL ? 0 : val_size(elmt,ent_ty);
}
uint64_t space_of_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt,val_entry_type ent_ty){
	uint64_t sum = 0;
	for(size_t i = start_elmt; i < end_elmt; ++i){
		sum += space_of_element(cache,i,ent_ty);
	}
	return sum;
}
uint32_t reported_space_of_element(cache_t cache,uint64_t elmt){
	load_key_to_intarr(elmt);
	uint32_t out_size = 0;
	val_type null_val = cache_get_wrapper(cache,keyarr,&out_size);
	return null_val == NULL ? 0 : out_size;
}
uint64_t reported_space_of_elements(cache_t cache,uint64_t start_elmt,uint64_t end_elmt){
	uint64_t sum = 0;
	for(size_t i = start_elmt; i < end_elmt; ++i){
		sum += reported_space_of_element(cache,i);
	}
	return sum;
}
bool element_exists(cache_t cache,uint64_t elmt){
	load_key_to_intarr(elmt);
	uint32_t null_size = 0;
	val_type val = cache_get_wrapper(cache,keyarr,&null_size);
	return val != NULL;
}
bool elements_exist(cache_t cache,uint64_t start_elmt,uint64_t end_elmt){
	// Only returns true when all elements exist
	bool res = true;
	for(uint64_t i = start_elmt; i < end_elmt; ++i){
		res = res && element_exists(cache,i);
	}
	return res;
}
bool elements_dont_exist(cache_t cache,uint64_t start_elmt,uint64_t end_elmt){
	// Only returns true if all elements do not exist
	for(uint64_t i = start_elmt; i < end_elmt; ++i){
		if (element_exists(cache,i)) return false;
	}
	return true;
}

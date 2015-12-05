#ifndef ARTHOOK_MANAGER_H
#define ARTHOOK_MANAGER_H

#include <sys/types.h>
#include <pthread.h>
#include "utils.h"
#include "globals.h"

#define SIZE 256

typedef struct jni_cache_t{
    char key[SIZE];
    jclass ref;
    UT_hash_handle hh;
}jni_cache_t;

int arthook_manager_init(JNIEnv*);
// arthook_manager
int add_hook(arthook_t*);
arthook_t* get_hook_by_key(char* key);
int is_method_in_hashtable();
void* get_method_from_hashtable(unsigned int* target);
void print_hashtable();
static int add_cache(jni_cache_t*);
void create_cache(char* name, void *ref);
void* check_cache(char* name);
#endif


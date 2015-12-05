/*
 * Functions for hooks data structure management
 *
 */

#include "arthook_manager.h"

static struct arthook_t *h = NULL;
static struct jni_cache_t *jnicache = NULL;

pthread_rwlock_t lock;
pthread_rwlock_t cachelock;

void create_cache(char* name, jclass ref)
{
    if(check_cache(name) != 0) {
        arthooklog("%s ERRORE cache item %s gia presente!!\n", __PRETTY_FUNCTION__, name);
        return;
    }
    arthooklog("%s chiamata con: %s = 0x%08x\n", __PRETTY_FUNCTION__, name, ref);
    jni_cache_t* tmp = (jni_cache_t*) malloc(sizeof(struct jni_cache_t));
    //memcpy(tmp->ref, ref, 4);
    tmp->ref = ref;
    strncpy(tmp->key, name, sizeof(tmp->key));
    tmp->key[strlen(tmp->key)+1] = 0x0;
    arthooklog("%s addedd %s \n", __PRETTY_FUNCTION__, tmp->key);
    add_cache(tmp);
}
static int add_cache(jni_cache_t* new){
    if (pthread_rwlock_wrlock(&cachelock) != 0) return 1;
    arthooklog("cache added: %s \n", new->key);
    HASH_ADD_STR(jnicache, key, new);
    pthread_rwlock_unlock(&cachelock);
}
void* check_cache(char* name)
{
    arthooklog("%s chiamato con : %s \n", __PRETTY_FUNCTION__, name);
    jni_cache_t* tmp = NULL;
    for(tmp = jnicache; tmp != NULL; tmp = tmp->hh.next){
        arthooklog("cache item: %s = %s \n", tmp->key, name);
        if(strcmp(name, tmp->key) == 0) {
            arthooklog("%s ritorno 0x%08x \n", __PRETTY_FUNCTION__, tmp->ref);
            return tmp->ref;
        }
    }
    return 0;
}
int arthook_manager_init(JNIEnv* env)
{
    if (pthread_rwlock_init(&lock,NULL) != 0) return 1;
    if (pthread_rwlock_init(&cachelock,NULL) != 0) return 1;
}

int add_hook(arthook_t* new)
{
    if (pthread_rwlock_wrlock(&lock) != 0) return 1;
    HASH_ADD_STR(h, key, new);
    pthread_rwlock_unlock(&lock);
}
arthook_t* get_hook_by_key(char* key)
{
    arthook_t* myhook = NULL;
    if (pthread_rwlock_rdlock(&lock) != 0) return NULL;
    HASH_FIND_STR(h, key, myhook);
    pthread_rwlock_unlock(&lock);
    return myhook;
}

void print_hashtable(){
    arthook_t* tmp = NULL;
    for(tmp = h; tmp != NULL; tmp = tmp->hh.next){
        arthooklog("item: %s \n", tmp->clsname);
        arthooklog("original mid : %p \n",  tmp->original_meth_ID);
    }    
}

void* get_method_from_hashtable(unsigned int* target)
{
    arthook_t* tmp = NULL;
    int res = -1;
    for(tmp = h; tmp != NULL; tmp = tmp->hh.next){
        arthooklog("item : %s \n", tmp->clsname);
        arthooklog("original  mid: %p \n", tmp->original_meth_ID);
        arthooklog("sto cercando : 0x%08x \n", *target);
        res = memcmp(&tmp->original_meth_ID, target,4);
        arthooklog("memcmp res: %d \n", res);
        if(res == 0)
            return tmp;
    }
    return NULL;
}

int is_method_in_hashtable(unsigned int* target)
{
    arthook_t* tmp = NULL;

    for(tmp = h; tmp != NULL; tmp = tmp->hh.next){
        arthooklog("item: %s \n", tmp->clsname);
        arthooklog("original mid : %x \n", (unsigned int) tmp->original_meth_ID);
        arthooklog("searching mid : %x \n", (unsigned int)  *target);
        int res = memcmp(&tmp->original_meth_ID, target,4);
        arthooklog("memcpm res: %d \n", res);
        if(res == 0)
            return 1;
    }
    return 0;
}




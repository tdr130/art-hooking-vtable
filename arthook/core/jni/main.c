//
// Created by vaioco on 12/01/16.
//


#include <stdio.h>
#include "main.h"

//TODO check if it is per-thread
static JNIEnv* th_env = NULL;

int done = 0;
struct hook_t invokeh;
struct hook_t eph ;
pthread_mutex_t epoll_lock ;

/*
 * set hooks from target dict
 */
static int hookTargetMethod(meth_hooks_p target){
    arthooklog("%s called with target = %s \n", __PRETTY_FUNCTION__, target->key);
    JNIEnv* myenv = get_global_jnienv();
    jobject dexLoader = get_dexloader();
    if(!myenv || !dexLoader){
        LOGG("%s ERROR get_dexloader!!\n", __PRETTY_FUNCTION__);
        return 1;
    }
    arthooklog("%s cerco class: %s \n", __PRETTY_FUNCTION__, target->hookclsname);
    jclass mycls = findClassFromClassLoader(myenv,dexLoader, target->hookclsname);
    if(!mycls){
        LOGG("ERROR findclassfromclassloader!!\n");
        return 1;
    }
    arthooklog("%s trovata classe %x\n", __PRETTY_FUNCTION__, mycls);
    jmethodID testID = (*myenv)->GetMethodID(myenv,mycls,target->mname, target->msig);
    if( jni_check_for_exception(myenv) ){
        LOGG("ERROR cannot find method %s \n", target->mname);
        return 1;
    }
    arthook_t* tmp = create_hook(myenv,target->cname, target->mname, target->msig, mycls,testID);
    if(!tmp){
        LOGG("ERROR create_hook\n");
        return 1;
    }
    add_hook(tmp);
}
/* Entrypoint */
struct config_t* arthook_entrypoint_start(char *config_fname, bool zygote){
    arthooklog("%s called \n", __PRETTY_FUNCTION__ );
    /* set global JNIEnv */
    th_env = get_jnienv();
    if( th_env == NULL ){
        LOGG("ERROR getting JNIEnv* \n");
        return NULL;
    }
    /* read JSON config file into config_t struct */
    config_t* myconfig =  (struct config_t*) config_init(config_fname, zygote);
    if(myconfig == NULL){
        LOGG("ERROR creating configuration \n");
        return NULL;
    }
    /* init data structures mutex */
    if(arthook_manager_init(th_env)) {
        LOGG("ERROR on manager init!!\n");
        return NULL;
    }
    /* set native hook on InvokeMethod.
     * used for trigger the hooking step
     * epoll_wait is broken.
     */
    if(myconfig->osversion <= 19 ){
        if(hook(&invokeh, getpid(), "libart.",
                "_ZN3art12InvokeMethodERKNS_18ScopedObjectAccessEP8_jobjectS4_S4_",
                NULL, my_invoke_method) == 0){
            LOGG("cannot find symbol _ZN3art12InvokeMethodERKNS_18ScopedObjectAccessEP8_jobjectS4_S4_!!\n");
            return;
        }
    }
    else{
        if(hook(&invokeh, getpid(), "libart.",
                "_ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b",
                NULL, my_invoke_method) == 0){
            LOGG("cannot find symbol _ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b!!\n");
            return;
        }
    }

    //TODO return void
    return myconfig;
}
/* register native methods exposed to Java */

/*
 */
int arthook_init(){
    arthooklog("%s called \n", __PRETTY_FUNCTION__);
    JNIEnv* myenv = get_global_jnienv();
    configT_ptr  configuration = getConfig();
    if(configuration->zygote){
        char *work_dir = _config_create_env();
        if (work_dir == 0) {
            LOGG("ERROR CREATE ENV\n");
            return 1;
        }
        arthooklog("working dir: %s , len = %d \n ", work_dir, strlen(work_dir));
        configuration->optdir = (char*) calloc(strlen(work_dir) + 1, sizeof(char));
        strncpy(configuration->optdir,work_dir,strlen(work_dir));
        configuration->optdir[strlen(work_dir)] = 0x0;
    }
    jobject dexloader = set_dexloader(myenv, MYDEX);
    if(dexloader == NULL){
        LOGG("ERROR dexloader!!\n");
        return 1;
    }
    jclass hookcls = load_class_from_dex(myenv, dexloader, MYHOOKCLASS);
    if(  hookcls == NULL){
        LOGG("%s ERROR loading class: %s !!\n", __PRETTY_FUNCTION__,MYHOOKCLASS);
        return 1;
    }

    targetMethodsListIterator(hookTargetMethod);

    print_hashtable();
    arthook_entrypoint_end(hookcls);
    arthooklog("[ %s ]  init terminated, happy hooking !! \n", __PRETTY_FUNCTION__);
    return 0;
}

void* my_invoke_method(void* soa, jobject javaMethod, void* javaReceiver, jobject javaArgs) {
    arthooklog("!!! %s receiver : 0x%08x, javamethod : 0x%08x \n", __PRETTY_FUNCTION__,
               (unsigned int) javaReceiver, (unsigned int) javaMethod);

    void *(*orig_invoke_method)(void *soa, void *javaMethod, void *javaReceiver, void *javaArgs);
    void *res = NULL;
    orig_invoke_method = (void *) invokeh.orig;
    if (done == 0) {
        if (arthook_init()) {
            LOGG("ERROR on %s\n", __PRETTY_FUNCTION__);
            return 0;
        }
        done = 1;
    }
    arthooklog("called method is not an hooked method, return to normal flow \n");
    hook_precall(&invokeh);
    res = orig_invoke_method(soa, javaMethod, javaReceiver, javaArgs);
    //hook_postcall(&invokeh);
    arthooklog("end originalcall\n");

    return res;
}
void* arthook_entrypoint_end(jclass mycls){
    if( arthook_bridge_init( get_global_jnienv(), (jclass) mycls)){
        LOGG("ERROR arthookbridge: register native methods \n");
        return NULL;
    }
}
JNIEnv* get_global_jnienv(){
    return th_env;
}
void set_hookdemo_init(void* func){
    hookdemo_init = func;
}

    /*
    jint checkstack = 0;
    void *checkcalledmethod = (void *) 0;
    JNIEnv *myenv = get_global_jnienv();
    if(myenv == NULL){
        LOGG("ERROR getting JNIEnv* \n");
        goto error;
    }
    checkstack = printStackTraceFromJava(myenv);

    // check if an hooked method is the target of the reflection call
    checkcalledmethod = hh_check_javareflection_call(myenv, javaMethod, javaReceiver);
    // checks:
    // 1. the method called with reflection is hooked?
    // 2. the call is from our patch method?
    //
    //checkcalledmethod = 0;
    // called method is not an hooked method
    if(!checkcalledmethod){
        arthooklog("called method is not an hooked method, return to normal flow \n");
        hook_precall(&invokeh);
        res = orig_invoke_method(soa,javaMethod,javaReceiver,javaArgs);
        //hook_postcall(&invokeh);
        arthooklog("end originalcall\n");
    }
    else{
        if(checkstack){
            // trapped call is from a "patch method"
            // so we have to direct call the original method
            arthooklog("trapped call is from trusted patch code, calling original method \n");
            return callOriginalReflectedMethod(myenv, javaReceiver, (arthook_t*) checkcalledmethod, javaArgs);
        }
        else{
            // call the "patch method"
            //
            arthooklog("ok hooked method founded, calling patch method \n");
            return call_patch_method(myenv, (arthook_t*) checkcalledmethod, javaReceiver, javaArgs);
        }
    }
    return res;

    error:
    hook_precall(&invokeh);
    return 0;
}
     */

//NOT USED, IS BROKEN!!!
/*
int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    if (pthread_mutex_lock(&epoll_lock) != 0) return 1;
    arthooklog("inside %s \n", __PRETTY_FUNCTION__);
    int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
    orig_epoll_wait = (void*)eph.orig;
    hook_precall(&eph);
    int res = orig_epoll_wait(epfd, events, maxevents, timeout);
    if(!done) {
        //init_hook();
        //_ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b
        hook(&invokeh, getpid(), "libart.", "_ZN3art12InvokeMethodERKNS_18ScopedObjectAccessEP8_jobjectS4_S4_",
             my_epoll_wait_arm, my_invoke_method);
        done = 1;
    }
    pthread_mutex_unlock(&epoll_lock);
    return res;
}
*/
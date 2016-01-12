//
// Created by vaioco on 12/01/16.
//


#include "main.h"

static struct config_t* myconfig;

struct config_t* arthook_entrypoint(char* config_fname){
    myconfig =  (struct config_t*) config_init(config_fname);
    if(myconfig == NULL) return 0;
    return myconfig;
}


int done = 0;
struct hook_t invokeh;
struct hook_t eph ;
pthread_mutex_t epoll_lock ;

void set_hookdemo_init(void* func){
    hookdemo_init = func;
}

int init_hook(JNIEnv* env)
{
    arthooklog("dentro %s ... \n", __PRETTY_FUNCTION__);
    if(hookdemo_init(env) == 1){
        return 1;
    }
    return 0;
}

//NOT USED!!!
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

void* my_invoke_method(void* soa, jobject javaMethod, void* javaReceiver, jobject javaArgs){
    arthooklog("!!! %s receiver : 0x%08x, javamethod : 0x%08x \n", __PRETTY_FUNCTION__,
               (unsigned int) javaReceiver,  (unsigned int) javaMethod);

    void* (*orig_invoke_method)(void* soa, void* javaMethod, void* javaReceiver, void* javaArgs);
    void* checkcalledmethod = (void*) 0;
    void* res = NULL;
    jint checkstack = 0;

    orig_invoke_method = (void*) invokeh.orig;

    JNIEnv* th_env = get_jnienv();
    if(!th_env){
        LOGG("ERROR getting JNIEnv* \n");
        goto error;
    }
    arthooklog("env  =  %p \n", th_env);

    if(done==0) {
        if(init_hook(th_env)){
            LOGG("ERROR on %s\n", __PRETTY_FUNCTION__);
            goto error;
        }
        done=1;
    }

    checkstack = printStackTraceFromJava(th_env);

    // check if an hooked method is the target of the reflection call
    checkcalledmethod = hh_check_javareflection_call(th_env, javaMethod, javaReceiver);
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
            return callOriginalReflectedMethod(th_env, javaReceiver, (arthook_t*) checkcalledmethod, javaArgs);
        }
        else{
            // call the "patch method"
            //
            arthooklog("ok hooked method founded, calling patch method \n");
            return call_patch_method(th_env, (arthook_t*) checkcalledmethod, javaReceiver, javaArgs);
        }
    }
    return res;

    error:
    hook_precall(&invokeh);
    return 0;
}


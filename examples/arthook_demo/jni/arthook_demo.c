#include <stdio.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <jni.h>
#include <unistd.h>
#include "arthook_demo.h"

void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{

    bool zygote = 0;
    if (pthread_mutex_init(&lock,NULL) != 0) return;
    // adbi and arthook log functions
    set_logfunction(my_log);
    set_arthooklogfunction(artlogmsgtofile);

    //set_hookdemo_init(my_hookdemo_init);
    arthooklog("ARTDroid %s started pid = %d \n", __FILE__, getpid());

    // resolve libart.so symbols used by artstuff.c
    if(resolve_symbols(&d) ){
        LOGG("cannot resolve symbols from libart.so!!\n");
        return;
    }
    if( processIsZygote() ){
        zygote = 1;
        arthooklog("%s zygote!!\n", __PRETTY_FUNCTION__);
    }
    //TODO
    configT_ptr configuration =  arthook_entrypoint_start("/data/local/tmp/test.json", zygote);
    if( configuration == NULL){
        LOGG("ERROR CONFIGURATION INIT!!\n");
        return;
    }

    // hook native functions
    //hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait_arm, my_epoll_wait);
    //init_hook();
    arthooklog("running on api version %d \n", configuration->osversion);
    arthooklog("%s  ended pid %d \n\n", __PRETTY_FUNCTION__, getpid());
}


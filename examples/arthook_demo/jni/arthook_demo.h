#ifndef HOOK_DEMO_H
#define HOOK_DEMO_H

#include "arthook_t.h"
#include "arthook_bridge.h"
#include "arthook_manager.h"
#include "arthook_helper.h"
#include "artstuff.h"

//#include "nativehooks.h"
#include "base.h"


#define MYDEX  "/data/local/tmp/dex/target.dex"
#define MYOPTDIR  "/data/local/tmp/dex/opt"


#define MYHOOKCLASS "org/sid/arthookbridge/HookCls"

typedef struct{
    char* cname;
    char* mname;
    char* msig;
    char* hookclsname;
    char* hookmname;
    char* hookmsig;
    jmethodID target;
} WrapMethodsToHook;

extern int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
extern void* my_invoke_method(void* soa, jobject javaMethod, void* javaReceiver, jobject javaArgs);
extern void set_hookdemo_init(void* func);
extern struct hook_t invokeh;
extern struct hook_t eph;
extern pthread_mutex_t lock;

int my_hookdemo_init(JNIEnv*);

#endif
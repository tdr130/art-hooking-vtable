#ifndef HOOK_DEMO_H
#define HOOK_DEMO_H

#include "arthook_t.h"
#include "arthook_bridge.h"
#include "arthook_manager.h"
#include "arthook_helper.h"
#include "artstuff.h"

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

extern void set_hookdemo_init(void* func);;
extern pthread_mutex_t lock;
extern void my_log(char *msg);
extern void artlogmsgtofile(char* msg);
int my_hookdemo_init();

#endif
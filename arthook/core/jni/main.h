//
// Created by vaioco on 12/01/16.
//

#ifndef ART_HOOKING_VTABLE_MAIN_H
#define ART_HOOKING_VTABLE_MAIN_H

#include "arthook_t.h"
#include "arthook_bridge.h"
#include "arthook_manager.h"
#include "arthook_helper.h"
#include "artstuff.h"
#include "hook.h"
#include "config.h"
#include <stdbool.h>

int (*hookdemo_init)();

/*
extern struct hook_t invokeh;
extern struct hook_t eph;
extern pthread_mutex_t lock;
*/
extern int my_epoll_wait_arm(int ,struct epoll_event *, int , int );
JNIEnv* get_global_jnienv();
struct config_t* arthook_entrypoint_start(char *config_fname, bool);
void* arthook_entrypoint_end(void* mycls);
void* my_invoke_method(void* , jobject , void* , jobject );
void set_hookdemo_init(void* );
//int my_epoll_wait(int, struct epoll_event *, int , int );

#endif //ART_HOOKING_VTABLE_MAIN_H

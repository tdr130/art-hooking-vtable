#ifndef HOOK_DEMO_H
#define HOOK_DEMO_H

#include "arthook_t.h"
#include "arthook_bridge.h"
#include "arthook_manager.h"
#include "arthook_helper.h"
#include "artstuff.h"
#include "base.h"

extern pthread_mutex_t lock;
extern void my_log(char *msg);
extern void artlogmsgtofile(char* msg);

#endif
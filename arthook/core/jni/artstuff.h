#ifndef ARTSTUFF_H
#define ARTSTUFF_H

#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <jni.h>
#include <stdio.h>

#include "arthook_t.h"
#include "globals.h"

struct artstuff_t d;


typedef jint (*JNI_GetCreatedJavaVMs_func)(void**,jsize,jsize*);
typedef void* (*art_th_currentFromGdb_func)();
typedef void (*art_dbg_SuspendVM_func)();
typedef void (*art_dbg_SuspendSelf_func)();
typedef void (*art_thlist_resumeAll_func)();
typedef void (*art_dbg_ResumeVM_func)();
typedef void (*art_dumpallclasses_func)(int);
typedef void (*art_printloadedclasses_func)(int);



struct artstuff_t{
	void* art_hand;
	void* art_runtime_instance;
	JNI_GetCreatedJavaVMs_func JNI_GetCreatedJavaVMs_fnPtr;
    art_th_currentFromGdb_func art_th_currentFromGdb_fnPtr;
    art_dbg_SuspendVM_func art_dbg_SuspendVM_fnPtr ;
    art_dbg_SuspendSelf_func art_dbg_SuspendSelf_fnPtr;
    art_thlist_resumeAll_func art_thlist_resumeAll_fnPtr;
    art_dbg_ResumeVM_func  art_dbg_ResumeVM_fnPtr;
    art_dumpallclasses_func  art_dumpallclasses_fnPtr;
    art_printloadedclasses_func art_printloadedclasses_fnPtr;
};

void _GetCreatedJavaVMs(struct artstuff_t*,void**,jsize, jsize*);
jclass _art_jni_FindClass(struct artstuff_t* , JNIEnv* , const char* );
void _suspendAllForDbg(struct artstuff_t *);
int resolve_symbols(struct artstuff_t *);
void call_test_asm(struct artstuff_t *d);
#endif

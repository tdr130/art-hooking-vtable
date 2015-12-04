#ifndef ARTHOOK_HELPER_H
#define ARTHOOK_HELPER_H

#include "arthook_t.h"
#include "utils.h"
#include "globals.h"
#include "arthook_manager.h"
#include "arthook_bridge.h"

#define CLAZZ_OFF_ 0x8
#define VMETHODS_LEN_OFF_  0x8

#define KK_VMETHODS_OFF 0x30
#define KK_VTABLE_OFF 0x34
#define KK_ACCESS_FLAG_OFF 0x1c
#define KK_CSIZE_OFF 0x3c

// >= LOLLIPOP
#define LOLLIPOP_VTABLE_OFF 0x38
#define LOLLIPOP_VMETHODS_OFF 0x34
#define LOLLIPOP_VMETHODS_FIRST 0xc
#define LOLLIPOP_MID_INDEX_OFF 0x20


typedef struct art_offsets
{
    int CLAZZ_OFF;
    int METH_INDEX_OFF;
    int VTABLE_OFF;
    int VMETHODS_OFF;
    int VMETHODS_LEN_OFF;
    int VMETHODS_FIRST_OFF;
} lol_offsets;

typedef struct art_kk_offsets
{
    int CLAZZ_OFF;
    int VTABLE_OFF;
    int VMETHODS_OFF;
    int VMETHODS_LEN_OFF;
    int ACCESS_FLAG_OFF;
    int CSIZE_OFF;
} kk_offsets;

arthook_t* create_hook(JNIEnv *, char *, const char* , const char*, jclass,jmethodID);
static unsigned int* searchInMemoryVtable(unsigned int , int , int, bool );
static unsigned int* searchInMemoryVmeths(unsigned int , int , int );
void* hh_check_javareflection_call(JNIEnv *, jobject, jobject );
void* callOriginalReflectedMethod(JNIEnv* env, jobject javaReceiver, arthook_t* tmp, jobject);
jobject call_patch_method(JNIEnv* env, arthook_t* h, jobject thiz, jobject);

#endif
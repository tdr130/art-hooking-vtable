#ifndef UTILS_H
#define UTILS_H

#include <android/log.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "modifiers.h"
#include "globals.h"
#include "uthash.h"
#include "jni_helper.h"
#include "arthook_bridge.h"

#undef NELEM
#define NELEM(x)            (sizeof(x)/sizeof(*(x)))

#define MAXSIZE 512



jint getAPIVersion();
void set_pointer(unsigned int *, unsigned int );
jmethodID getMethodID(JNIEnv *,jclass , char *, char* );
jmethodID fromObjToMethodID(JNIEnv *, jobject , char *, char *);
int isLollipop();
jclass _findClass(JNIEnv* , char* );
jobject createInstanceFromClsName(JNIEnv* , char* );
jobject createInstanceFromJClsName(JNIEnv* , jstring );
char* getCharFromJstring(JNIEnv* , jstring );
char* parseSignature(JNIEnv* env, char* sig, jvalue* args, jobjectArray joa, int counter);
jobject parseReturnType(JNIEnv* env, char *sig, jobject thiz, jclass c, jmethodID mid, jvalue* args);
char* _getprop(char* command, char*);
#endif

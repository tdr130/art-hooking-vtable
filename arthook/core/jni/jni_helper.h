#ifndef JNI_WRAPPERS_H

#define JNI_WRAPPERS_H

#include "utils.h"
#include "globals.h" 
#include "artstuff.h"

JNIEnv* getEnv();

jobject createDexClassLoader(JNIEnv* , jobject, char*, char* );

jobject getSystemClassLoader(JNIEnv*);

jclass findClassFromClassLoader(JNIEnv* , jobject , char*);

jclass loadClassFromClassLoader(JNIEnv* , jobject , char*);

int jni_check_for_exception(JNIEnv* env);

#endif

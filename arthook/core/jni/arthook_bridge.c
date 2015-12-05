/*
 * Contains functions exposed to the Java side
 *
 *
 */

#include "arthook_bridge.h"

static jobject gDexLoader;

jobject get_dexloader()
{
    return gDexLoader;
}

jclass load_class_from_dex(JNIEnv* env, jobject dexloader, char* clsname)
{
    jclass c = loadClassFromClassLoader(env, dexloader,clsname );
    return c;
}
jobject set_dexloader(JNIEnv* env, char* dexfile, char* optdir)
{
    jobject systemCL = getSystemClassLoader(env);
    jobject dexloader  = createDexClassLoader(env, systemCL, dexfile, optdir);
    gDexLoader = (*env)->NewGlobalRef(env, dexloader);
    jclass c2 = loadClassFromClassLoader(env, dexloader, BRIDGE_UTILS );
    return gDexLoader;
}
//aggiungere una cache
jint printStackTraceFromJava(JNIEnv* env)
{
    jclass test = findClassFromClassLoader(env, gDexLoader, BRIDGE_UTILS);

    if(!test) return 0;
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "printStackTraces", "()I");
    jint res = (*env)->CallStaticIntMethod(env, test, mid);
    return res;
}
jboolean callGetBoolean(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getBoolean", "([Ljava/lang/Object;I)Z");
    jboolean res = (*env)->CallStaticIntMethod(env, test, mid, javaArgs, index);
    return res;

}
jdouble callGetDouble(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getDouble", "([Ljava/lang/Object;I)D");
    jdouble res = (*env)->CallStaticDoubleMethod(env, test, mid, javaArgs, index);
    return res;

}
jfloat callGetFloat(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getFloat", "([Ljava/lang/Object;I)F");
    jfloat res = (*env)->CallStaticFloatMethod(env, test, mid, javaArgs, index);
    return res;
}
jbyteArray callGetByteArray(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getByteArray", "([Ljava/lang/Object;I)[B");
    jbyteArray res = (*env)->CallStaticObjectMethod(env, test, mid, javaArgs, index);
    return res;
}
jlong callGetLong(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getLong", "([Ljava/lang/Object;I)J");
    jlong res = (*env)->CallStaticFloatMethod(env, test, mid, javaArgs, index);
    return res;

}
jint callGetInt(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getInt", "([Ljava/lang/Object;I)I");
    jint res = (*env)->CallStaticIntMethod(env, test, mid, javaArgs, index);
    return res;

}
jobject callGetObj(JNIEnv* env, jobject javaArgs, int index)
{
    jclass test = findClassFromClassLoader(env,gDexLoader,BRIDGE_UTILS);
    jmethodID mid = (*env)->GetStaticMethodID(env, test, "getObj", "([Ljava/lang/Object;I)Ljava/lang/Object;");
    jobject res = (*env)->CallStaticObjectMethod(env, test, mid, javaArgs, index);
    return (*env)->NewGlobalRef(env,res);

}
//arthook_t* tmp = create_hook(env,methodsToHook[i].cname, methodsToHook[i].mname, methodsToHook[i].msig, gtest,testID);
jobject create_hook_from_java(JNIEnv* env,jstring _cname, jstring _mname,jstring _msig, \
                jstring _patchClass,  jstring _patchMethod){
    char * cname =  getCharFromJstring(env, _cname);
    char * mname =  getCharFromJstring(env, _mname);
    char * msig =  getCharFromJstring(env, _msig);
    //char * patchClass =  getCharFromJstring(env, _patchClass);
    //char * patchMethod =  getCharFromJstring(env, _patchMethod);
    arthooklog("diocane chiamato da java\n");
/*
    test = findClassFromClassLoader(env,dexloader, methodsToHook[i].hookclsname);
    jclass gtest = (*env)->NewGlobalRef(env, test);
    jmethodID testID = (*env)->GetStaticMethodID(env,gtest,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
    arthook_t* tmp = create_hook(env,methodsToHook[i].cname, methodsToHook[i].mname, methodsToHook[i].msig, gtest,testID);
    add_hook(tmp);
*/
}

static JNINativeMethod artHookMethods[] = {
    /* name, signature, funcPtr */ 
    //create_hook wrapper
    {"createHookFromJava",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            (void*) create_hook_from_java },
};
static int jniRegisterNativeMethods(JNIEnv* env, jclass cls)
{
    if ((*env)->RegisterNatives(env, cls, artHookMethods, NELEM(artHookMethods)) < 0) {
        return 1;
    }
    return 0;
}
int arthook_bridge_init(JNIEnv* env, jclass cls){
    jniRegisterNativeMethods(env,cls);
}




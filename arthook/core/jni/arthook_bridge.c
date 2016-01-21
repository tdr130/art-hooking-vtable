/*
 * Contains functions exposed to the Java side
 * and helper functions used to call Java functions
 *
 */

#include "arthook_bridge.h"
#include "config.h"

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
jobject set_dexloader(JNIEnv* env, char* dexfile)
{
    configT_ptr c = getConfig();
    arthooklog("%s CALLED with optdir = %s \n", __PRETTY_FUNCTION__, c->optdir);
    jobject systemCL = getSystemClassLoader(env);
    jobject dexloader  = createDexClassLoader(env, systemCL, dexfile, c->optdir);
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
static void _callOriginalMethod(JNIEnv* env, jobject obj, jstring key, jobject thiz,
                                jbyteArray jba, jint offset, jint size) {
    char *mykey = getCharFromJstring(env, key);
    arthooklog("%s key = %s\n", __PRETTY_FUNCTION__, mykey);
    arthook_t *target = get_hook_by_key(mykey);
    jclass c = (*env)->GetObjectClass(env, thiz);
    (*env)->CallNonvirtualObjectMethod(env, thiz, c, target->original_meth_ID, jba, offset, size);
}
void callOriginalVoidMethod(JNIEnv* env, jobject thiz,jclass c, jmethodID mid, jvalue* args)
{
    arthooklog("%s\n", __PRETTY_FUNCTION__);
    (*env)->CallNonvirtualVoidMethodA(env, thiz, c, mid, args);
}
jobject callOriginalObjectMethod(JNIEnv* env, jobject thiz,jclass c, jmethodID mid, jvalue* args)
{
    arthooklog("%s\n", __PRETTY_FUNCTION__);
    return (*env)->CallNonvirtualObjectMethodA(env, thiz, c, mid, args);
}
static jobject _callOriginalMethod2(JNIEnv* env, jobject obj, jstring key, jobject thiz,
                                jobjectArray joa) {
    /*
        jsize i = 0;
    jint k,z;
    jbyteArray jba;
    va_list list;
    va_start(list, numargs);

    for(i=0;i<numargs;i++){
        if(i==0) {
            jba = va_arg(list, jobject);
            args[i].l = jba;
            arthooklog("%s jvalue[%d] = %x \n", __PRETTY_FUNCTION__, i, jba);
        }
        else {
            k = va_arg(list, jint);
            args[i].i = k;
            arthooklog("%s jvalue[%d] : %d  = %d \n", __PRETTY_FUNCTION__, i, k, args[i].i);
        }
    }
    va_end( list );
    return;
    */
    char *mykey = getCharFromJstring(env, key);
    arthook_t *target = get_hook_by_key(mykey);
    if(!target) return NULL;
    arthooklog("%s key = %s\n", __PRETTY_FUNCTION__, mykey);
    jvalue* p = tryToUnbox(env, target, joa, thiz, false);
    arthooklog("%s after try to unbox \n", __PRETTY_FUNCTION__);
    jclass c = (*env)->GetObjectClass(env, thiz);
    arthooklog("%s checkreturntype: %s \n", __PRETTY_FUNCTION__, target->msig);
    jobject o = parseReturnType(env,target->msig,thiz,c,target->original_meth_ID, p);
    if(o != NULL)
        return (*env)->NewGlobalRef(env, o);
    //(*env)->CallNonvirtualVoidMethodA(env, thiz, c, target->original_meth_ID, p);
}
static JNINativeMethod artHookMethods[] = {
    /* name, signature, funcPtr */ 
    //create_hook wrapper
        /*
    {"createHookFromJava",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            (void*) create_hook_from_java },
            */
    {"callOriginalMethod",
            "(Ljava/lang/String;Ljava/lang/Object;[BII)V",
            (void*) _callOriginalMethod },
    {"callOriginalMethod2",
            "(Ljava/lang/String;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;",
            (void*) _callOriginalMethod2 },
};
static int jniRegisterNativeMethods(JNIEnv* env, jclass cls)
{
    if ((*env)->RegisterNatives(env, cls, artHookMethods, NELEM(artHookMethods)) < 0) {
        return 1;
    }
    arthooklog("%s native methods registered!! \n", __PRETTY_FUNCTION__);
    return 0;
}
int arthook_bridge_init(JNIEnv* env, jclass cls){
    arthooklog("%s chiamato \n", __PRETTY_FUNCTION__);
    if( jniRegisterNativeMethods(env,cls) )
        return 1;
    else return 0;
}




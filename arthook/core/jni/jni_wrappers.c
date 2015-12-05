#include "jni_wrappers.h"

JNIEnv* getEnv() 
{
    JNIEnv *env;
    int status = vms[0]->GetEnv(vms,(void**)&env, JNI_VERSION_1_6);
    if(status < 0) {    
        status = vms[0]->AttachCurrentThread(vms, &env, NULL);
        if(status < 0) {        
            return NULL;
        }
    }
    return env;
}

extern JNIEnv* get_jnienv(){
	jsize vm_count = NULL;
	jsize size = NULL;
    if(vms == NULL)
    	_GetCreatedJavaVMs(&d, (void**) &vms, size, &vm_count);
    arthooklog("called getcreatedjavavms, results: %p , count = %d \n", vms[0], vm_count);
    JNIEnv* env = getEnv(vms);
    arthooklog("jnienv = 0x%08x \n", (unsigned int) env);
    return env;
}


jobject createDexClassLoader(JNIEnv* env, jobject classLoader, char* mydexpath, char* myoptdir)
{
    jthrowable exc;
    arthooklog("%s , pid: %d\n",__PRETTY_FUNCTION__, getpid());
    jclass dexclassloader_cls = (*env)->FindClass(env, "dalvik/system/DexClassLoader");
    arthooklog("dexclassloader class = %p \n", dexclassloader_cls);
    jmethodID constructor = (*env)->GetMethodID(env, dexclassloader_cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    arthooklog("init methodid = %p \n", constructor);
    char* dexPath = mydexpath;
    // il proprietario di questa dir deve essere l'app
    char* optDir = myoptdir;
    jstring dexPathJ = (*env)->NewStringUTF(env, dexPath);
    jstring optDirJ = (*env)->NewStringUTF(env, optDir);
    char* libraryPath = "";
    jobject dexloader = (*env)->NewObject(env, dexclassloader_cls, constructor, dexPathJ, optDirJ, NULL, classLoader);
    exc = (*env)->ExceptionOccurred(env);
    if(exc){
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
    }
    arthooklog("created dex loader = %p \n ", dexloader);
    return dexloader;
}

jobject getSystemClassLoader(JNIEnv* env)
{
    jclass classLoader = (*env)->FindClass(env, "java/lang/ClassLoader");
    arthooklog("classloader class =  %p \n ", classLoader);
    jmethodID getSystemCL = (*env)->GetStaticMethodID(env, classLoader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    jobject systemCL = (*env)->CallStaticObjectMethod(env, classLoader, getSystemCL);
    arthooklog("systemclassloader = %p \n ", systemCL);
    return systemCL;

}

jclass loadClassFromClassLoader(JNIEnv* env, jobject classLoader, char* targetName)
{
    jclass test = NULL;
    test = (jclass) check_cache(targetName);
    if(test != 0) return test;
    arthooklog(" %s, targetname = %s \n ",__PRETTY_FUNCTION__, targetName);
    jclass classLoader_cls = (*env)->FindClass(env,"java/lang/ClassLoader");
    jmethodID loadClass = (*env)->GetMethodID(env, classLoader_cls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    arthooklog("loadClass mid = %p \n", loadClass);
    //jstring name = (*env)->NewStringUTF(env,"test/sid/org/ndksample/HookCls");   
    jstring name = (*env)->NewStringUTF(env,targetName);   
    jclass loaded = (*env)->CallObjectMethod(env, classLoader, loadClass, name);
    arthooklog("loaded class = %p \n" , loaded);
    jclass globalref = (jclass) (*env)->NewGlobalRef(env, loaded);
    create_cache(targetName, globalref);
    return (jclass) globalref;
}
//problemi con le ref, ne fa troppe!!!!
/*
 *
I/DEBUG   (  311): backtrace:
I/DEBUG   (  311):     #00 pc 00010548  /system/lib/libc.so (syscall+28)
I/DEBUG   (  311):     #01 pc 000a8999  /system/lib/libart.so (art::ReaderWriterMutex::ExclusiveLock(art::Thread*)+328)
I/DEBUG   (  311):     #02 pc 001ba5a5  /system/lib/libart.so (art::JNI::NewGlobalRef(_JNIEnv*, _jobject*)+296)
I/DEBUG   (  311):     #03 pc 0000773d  /data/local/tmp/libarthookdemo.so (findClassFromClassLoader+172)
I/DEBUG   (  311):     #04 pc 00007867  /data/local/tmp/libarthookdemo.so (printStackTraceFromJava+14)
I/DEBUG   (  311):     #05 pc 00006fe1  /data/local/tmp/libarthookdemo.so (my_invoke_method+144)
I/DEBUG   (  311):     #06 pc 0020c8f9  /system/lib/libart.so (art::Constructor_newInstance(_JNIEnv*, _jobject*, _jobjectArray*, unsigned char)+196)
I/DEBUG   (  311):     #07 pc 0001b3a7  /data/dalvik-cache/arm/system@framework@boot.oat
 */
jclass findClassFromClassLoader(JNIEnv* env, jobject classLoader, char* targetName)
{
    jclass test = NULL;
    test = (jclass) check_cache(targetName);
    if(test != 0) {
        arthooklog("%s return from cache: 0x%08x\n", __PRETTY_FUNCTION__, test);
        return test;
    }
    jclass classLoader_cls = (*env)->FindClass(env,"java/lang/ClassLoader");
    jmethodID findClass = (*env)->GetMethodID(env, classLoader_cls, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    arthooklog("findClass mid = %p \n", findClass);
    jstring name = (*env)->NewStringUTF(env,targetName);   
    jclass res = (*env)->CallObjectMethod(env,classLoader,findClass,name);
    arthooklog("HookClass =  %p\n" , res);
    jclass globalref = (jclass) (*env)->NewGlobalRef(env, res);
    create_cache(targetName, globalref);
    return (jclass) globalref;
}


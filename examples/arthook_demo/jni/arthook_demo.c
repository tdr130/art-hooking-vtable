#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <jni.h>
#include <stdlib.h>

// adbi include
#include "base.h"

#include "arthook_demo.h"

static WrapMethodsToHook methodsToHook[] = {

    {"android/telephony/TelephonyManager","getDeviceId","()Ljava/lang/String;",
        MYHOOKCLASS, "getDeviceId", "(Ljava/lang/Object;)Ljava/lang/String;", NULL},
    /*
    {"android/app/ContextImpl","openFileOutput","(Ljava/lang/String;I)Ljava/io/FileOutputStream;",
            MYHOOKCLASS, "openFileOutput", "(Ljava/lang/Object;Ljava/lang/String;I)Ljava/io/FileOutputStream;", NULL},

    {"android/app/Activity","openFileOutput","(Ljava/lang/String;I)Ljava/io/FileOutputStream;",
        MYHOOKCLASS, "openFileOutput", "(Ljava/lang/Object;Ljava/lang/String;I)Ljava/io/FileOutputStream;", NULL},

    {"android/hardware/Camera","takePicture",
     "(Landroid/hardware/Camera$ShutterCallback;Landroid/hardware/Camera$PictureCallback;Landroid/hardware/Camera$PictureCallback;)V",
     MYHOOKCLASS, "takePicture",
            "(Ljava/lang/Object;Landroid/hardware/Camera$ShutterCallback;Landroid/hardware/Camera$PictureCallback;Landroid/hardware/Camera$PictureCallback;)V"
            , NULL},
    {"android/telephony/SmsManager", "sendTextMessage",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/app/PendingIntent;Landroid/app/PendingIntent;)V",
            MYHOOKCLASS, "sendTextMessage",
            "(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/app/PendingIntent;Landroid/app/PendingIntent;)V",
            NULL},
*/
};



int my_hookdemo_init(JNIEnv* env)
{
    arthooklog("dentro %s, starting....\n", __PRETTY_FUNCTION__);
    if(arthook_manager_init(env)){
        //ERRORS -> LOGCAT
        LOGG("ERROR on init!!\n");
        return 1;
    }
    jobject dexloader = set_dexloader(env, MYDEX, MYOPTDIR);
    jclass test = load_class_from_dex(env, dexloader, MYHOOKCLASS);
    //jclass test = findClassFromClassLoader(env,dexloader,MYHOOKCLASS );
    //if(jniRegisterNativeMethods(env, test) == 1 ){
    //    LOGG("JNI REGISTER NATIVE METHODS ERROR!!! \n");
    //}

    int i = 0;
    int nelem = NELEM(methodsToHook);
    for(i=0; i < nelem ; i++){
        test = findClassFromClassLoader(env,dexloader, methodsToHook[i].hookclsname);
        jclass gtest = (*env)->NewGlobalRef(env, test);
        jmethodID testID = (*env)->GetStaticMethodID(env,gtest,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
        arthook_t* tmp = create_hook(env,methodsToHook[i].cname, methodsToHook[i].mname, methodsToHook[i].msig, gtest,testID);
        add_hook(tmp);
    }    
    print_hashtable();
    arthooklog("[ %s ]  init terminated, happy hooking !! \n", __PRETTY_FUNCTION__);
    return 0;
}

static char logfile[] = "/data/local/tmp/adbi.log";

#undef log
#define log(...) \
        {FILE *fp = fopen(logfile, "a+"); if (fp) {\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}}

static int done = 0;
char artlogfile[]  = "/data/local/tmp/arthook.log";

JavaVM* vms = NULL;

// arm version of hook
extern int my_epoll_wait_arm(int epfd, struct epoll_event *events, int maxevents, int timeout);
extern void* my_invoke_method(void* soa, jobject javaMethod, void* javaReceiver, jobject javaArgs);

/*
 *  log function to pass to the hooking library to implement central loggin
 *
 *  see: set_logfunction() in base.h
 */
static void my_log(char *msg)
{
    log("%s", msg);
}
void artlogmsgtofile(char* msg){
    int fp = open(artlogfile, O_WRONLY|O_APPEND);
    if (fp != -1) {
        write(fp, msg, strlen(msg));
        close(fp);
    }
}

void* set_arthooklogfunction(void* func){
    void* old = log_function;
    log_function = func;
    return old;
}

void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
    if (pthread_mutex_init(&lock,NULL) != 0) return;
    // adbi and arthook log functions
    set_logfunction(my_log);
    set_arthooklogfunction(artlogmsgtofile);

    set_hookdemo_init(my_hookdemo_init);
    arthooklog("ARTDroid %s started\n", __FILE__);

    // resolve libart.so symbols used by artstuff.c
    resolve_symbols(&d);
    // hook native functions
    //hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait_arm, my_epoll_wait);
    //init_hook();
    hook(&invokeh, getpid(), "libart.", "_ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b", NULL, my_invoke_method);
    arthooklog("%s  ended\n", __PRETTY_FUNCTION__);
}


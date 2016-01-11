#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <jni.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arthook_demo.h"

static WrapMethodsToHook methodsToHook[] = {
        {"android/telephony/TelephonyManager","getDeviceId","()Ljava/lang/String;",
                MYHOOKCLASS, "getDeviceId", "()Ljava/lang/String;", NULL},
/*
    //<java.io.FileOutputStream: void write(byte[],int,int)> (FILE)
    {"java/io/FileOutputStream", "write", "([BII)V",
            MYHOOKCLASS, "write", "(Ljava/lang/Object;[BII)V", NULL},

    {"java/lang/String", "toString","()Ljava/lang/String;",
            MYHOOKCLASS, "toString", "(Ljava/lang/Object;)Ljava/lang/String;", NULL},
    {"java/security/MessageDigest", "update", "([BII)V",
            MYHOOKCLASS, "update", "(Ljava/lang/Object;[BII)V", NULL},
    {"android/webkit/WebView", "addJavascriptInterface", "(Ljava/lang/Object;Ljava/lang/String;)V",
            MYHOOKCLASS, "addJavascriptInterface", "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/String;)V", NULL},
    {"android/webkit/WebSettings", "setJavaScriptEnabled", "(Z)V",
            MYHOOKCLASS, "setJavaScriptEnabled", "(Ljava/lang/Object;Z)V", NULL},
    {"java/net/URL", "openConnection",
            "()Ljava/net/URLConnection;", MYHOOKCLASS, "openConnection",
            "(Ljava/lang/Object;)Ljava/net/URLConnection;", NULL},
    {"java/lang/String", "compareTo", "(Ljava/lang/String;)I",
            MYHOOKCLASS, "compareTo", "(Ljava/lang/Object;Ljava/lang/String;)I", NULL},
    {"android/app/ContextImpl","openFileOutput","(Ljava/lang/String;I)Ljava/io/FileOutputStream;",
            MYHOOKCLASS, "openFileOutput", "(Ljava/lang/Object;Ljava/lang/String;I)Ljava/io/FileOutputStream;", NULL},

    {"android/app/Activity", "startActivity", "(Landroid/content/Intent;)V",
            MYHOOKCLASS, "startActivity", "(Ljava/lang/Object;Landroid/content/Intent;)V", NULL },

    {"javax/net/ssl/HttpsURLConnection", "setHostnameVerifier","(Ljavax/net/ssl/HostnameVerifier;)V",
            MYHOOKCLASS, "setHostnameVerifier", "(Ljava/lang/Object;Ljavax/net/ssl/HostnameVerifier;)V", NULL},
    {"javax/net/ssl/HttpsURLConnection", "setSSLSocketFactory","(Ljavax/net/ssl/SSLSocketFactory;)V",
                    MYHOOKCLASS, "setSSLSocketFactory", "(Ljava/lang/Object;Ljavax/net/ssl/SSLSocketFactory;)V", NULL},
    {"android/app/SharedPreferencesImpl", "getString","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
            MYHOOKCLASS, "getString", "(Ljava/lang/Object;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", NULL},

    {"android/content/ContextWrapper", "startActivity", "(Landroid/content/Intent;Landroid/os/Bundle;)V",
            MYHOOKCLASS, "startActivity", "(Ljava/lang/Object;Landroid/content/Intent;Landroid/os/Bundle;)V", NULL },
    {"android/content/ContextWrapper", "startActivity", "(Landroid/content/Intent;)V",
            MYHOOKCLASS, "startActivity", "(Ljava/lang/Object;Landroid/content/Intent;)V", NULL },

    {"org/apache/http/conn/ssl/SSLConnectionSocketFactory", "<init>", "(Ljavax/net/ssl/SSLContext;)V",
      MYHOOKCLASS, "SSLConnectionSocketFactory", "(Ljava/lang/Object;Ljavax/net/ssl/SSLContext;)V", NULL},

    {"Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;",
            MYHOOKCLASS, "getMethod", "(Ljava/lang/Object;Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;",
            NULL},

    {"android/app/Activity","openFileOutput","(Ljava/lang/String;I)Ljava/io/FileOutputStream;",
        MYHOOKCLASS, "openFileOutput", "(Ljava/lang/Object;Ljava/lang/String;I)Ljava/io/FileOutputStream;", NULL},

    {"android/content/ContextWrapper", "sendBroadcast", "(Landroid/content/Intent;)V",
            MYHOOKCLASS, "sendBroadcast", "(Ljava/lang/Object;Landroid/content/Intent;)V", NULL },
    {"android/content/ContextWrapper", "sendBroadcast", "(Landroid/content/Intent;Ljava/lang/String;)V",
            MYHOOKCLASS, "sendBroadcast", "(Ljava/lang/Object;Landroid/content/Intent;Ljava/lang/String;)V", NULL },

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
                {"javax/crypto/Cipher", "doFinal", "([B)[B",
            MYHOOKCLASS, "doFinal", "(Ljava/lang/Object;[B)[B", NULL},
    {"javax/crypto/Cipher", "doFinal", "([BI)[B",
            MYHOOKCLASS, "doFinal", "(Ljava/lang/Object;[BI)[B", NULL},
    {"javax/crypto/Cipher", "doFinal", "([BII)[B",
            MYHOOKCLASS, "doFinal", "(Ljava/lang/Object;[BII)[B", NULL},
*/
};



int my_hookdemo_init(JNIEnv* env)
{
    char tmpdir[256];
    char ppid[8];
    jobject dexloader;
    jclass mycls;

    arthooklog("dentro %s, starting %d ....\n", __PRETTY_FUNCTION__, getpid());
    if(arthook_manager_init(env)){
        //ERRORS -> LOGCAT
        LOGG("ERROR on init!!\n");
        return 1;
    }
    sprintf(tmpdir,"%s/",MYOPTDIR);
    //zygote fix
    //entro dentro la cartella tmp_root e creo una mia dir
    if( chdir(MYOPTDIR) ){
        LOGG("ERROR chdir!!\n");
        return 1;
    }
    sprintf(ppid,"%d",getpid());
    strcat(tmpdir,ppid);
    arthooklog("creo working dir: %s \n", tmpdir);
    if( mkdir(tmpdir,S_IRWXU) ){
        LOGG("ERROR mkdir!!\n");
        return 1;
    }
    if( chdir(tmpdir) ){
        LOGG("ERROR chdir!!\n");
        return 1;
    }
    arthooklog("finito creazione dir\n");

    dexloader = set_dexloader(env, MYDEX, tmpdir);
    if(!dexloader){
        LOGG("ERROR dexloader!!\n");
        return 1;
    }
    if( load_class_from_dex(env, dexloader, MYHOOKCLASS)){
        LOGG("ERROR loading class: %s !!\n", MYHOOKCLASS);
        return 1;
    }
    //jclass test = findClassFromClassLoader(env,dexloader,MYHOOKCLASS );
    //if(jniRegisterNativeMethods(env, test) == 1 ){
    //    LOGG("JNI REGISTER NATIVE METHODS ERROR!!! \n");
    //}

    int i = 0;
    int nelem = NELEM(methodsToHook);

    for(i=0; i < nelem ; i++){
        mycls = findClassFromClassLoader(env,dexloader, methodsToHook[i].hookclsname);
        if(!mycls){
            LOGG("ERROR findclassfromclassloader!!\n");
            return 1;
        }
        arthooklog("%s trovata classe %x\n", __PRETTY_FUNCTION__, mycls);
        //jclass gtest = (*env)->NewGlobalRef(env, test);
        //jmethodID testID = (*env)->GetStaticMethodID(env,test,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
        jmethodID testID = (*env)->GetMethodID(env,mycls,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
        if( jni_check_for_exception(env) ){
            LOGG("ERROR cannot find method %s \n", methodsToHook[i].hookmname);
            return 1;
        }
        arthook_t* tmp = create_hook(env,methodsToHook[i].cname, methodsToHook[i].mname, methodsToHook[i].msig, mycls,testID);
        if(!tmp){
            LOGG("ERROR create_hook\n");
            return 1;
        }
        add_hook(tmp);
    }    
    print_hashtable();
    if( arthook_bridge_init(env, mycls)){
        LOGG("ERROR arthookbridge: register native methods \n");
        return 1;
    }
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
    if(resolve_symbols(&d) ){
        LOGG("cannot resolve symbols from libart.so!!\n");
        return;
    }

    // hook native functions
    //hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait_arm, my_epoll_wait);
    //init_hook();

    if(hook(&invokeh, getpid(), "libart.",
         "_ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b",
         NULL, my_invoke_method) == 0){
        LOGG("cannot find symbol _ZN3art12InvokeMethodERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectS4_S4_b!!\n");
        return;
    }

    arthooklog("%s  ended\n\n", __PRETTY_FUNCTION__);
}


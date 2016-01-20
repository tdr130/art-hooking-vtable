#include <stdio.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <jni.h>
#include <unistd.h>

#include "arthook_demo.h"


static WrapMethodsToHook methodsToHook[] = {
        {"android/telephony/TelephonyManager","getDeviceId","()Ljava/lang/String;",
                MYHOOKCLASS, "getDeviceId", "()Ljava/lang/String;", NULL},

    //<java.io.FileOutputStream: void write(byte[],int,int)> (FILE)
    {"java/io/FileOutputStream", "write", "([BII)V",
            MYHOOKCLASS, "write", "([BII)V", NULL},

    {"java/lang/String", "toString","()Ljava/lang/String;",
            MYHOOKCLASS, "toString", "()Ljava/lang/String;", NULL},
        /*
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

static struct config_t* configuration = NULL;


int my_hookdemo_init()
{

    jobject dexloader;
    jclass mycls;
    arthooklog("dentro %s, starting %d ....\n", __PRETTY_FUNCTION__, getpid());

    //configuration =  arthook_entrypoint_start("cippa");
    if( configuration == NULL){
        LOGG("ERROR CONFIGURATION INIT!!\n");
        return 1;
    }
    JNIEnv* myenv = get_global_jnienv();
    arthooklog("diomerda: %s\n", configuration->optdir);
    dexloader = set_dexloader(myenv, MYDEX, configuration);
    if(dexloader == NULL){
        LOGG("ERROR dexloader!!\n");
        return 1;
    }
    jclass hookcls = load_class_from_dex(myenv, dexloader, MYHOOKCLASS);
    if(  hookcls == NULL){
        LOGG("ERROR loading class: %s !!\n", MYHOOKCLASS);
        return 1;
    }
    int i = 0;
    int nelem = NELEM(methodsToHook);

    for(i=0; i < nelem ; i++){
        mycls = findClassFromClassLoader(myenv,dexloader, methodsToHook[i].hookclsname);
        if(!mycls){
            LOGG("ERROR findclassfromclassloader!!\n");
            return 1;
        }
        arthooklog("%s trovata classe %x\n", __PRETTY_FUNCTION__, mycls);
        //jclass gtest = (*env)->NewGlobalRef(env, test);
        //jmethodID testID = (*env)->GetStaticMethodID(env,test,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
        jmethodID testID = (*myenv)->GetMethodID(myenv,mycls,methodsToHook[i].hookmname, methodsToHook[i].hookmsig);
        if( jni_check_for_exception(myenv) ){
            LOGG("ERROR cannot find method %s \n", methodsToHook[i].hookmname);
            return 1;
        }
        arthook_t* tmp = create_hook(myenv,methodsToHook[i].cname, methodsToHook[i].mname, methodsToHook[i].msig, mycls,testID);
        if(!tmp){
            LOGG("ERROR create_hook\n");
            return 1;
        }
        add_hook(tmp);
    }    
    print_hashtable();
    arthook_entrypoint_end(hookcls);
    arthooklog("[ %s ]  init terminated, happy hooking !! \n", __PRETTY_FUNCTION__);
    return 0;
}



extern JavaVM* vms = NULL;

void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{

    if (pthread_mutex_init(&lock,NULL) != 0) return;
    // adbi and arthook log functions
    set_logfunction(my_log);
    set_arthooklogfunction(artlogmsgtofile);

    set_hookdemo_init(my_hookdemo_init);
    arthooklog("ARTDroid %s started pid = %d \n", __FILE__, getpid());

    // resolve libart.so symbols used by artstuff.c
    if(resolve_symbols(&d) ){
        LOGG("cannot resolve symbols from libart.so!!\n");
        return;
    }
    configuration =  arthook_entrypoint_start("dummy");
    if( configuration == NULL){
        LOGG("ERROR CONFIGURATION INIT!!\n");
        return;
    }

    // hook native functions
    //hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait_arm, my_epoll_wait);
    //init_hook();
    arthooklog("running on api version %d \n", configuration->osversion);
    arthooklog("%s  ended pid %d \n\n", __PRETTY_FUNCTION__, getpid());
}


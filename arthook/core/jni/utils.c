
#include "utils.h"


/*
    @name checkAPIVersion
    @brief return the API version number
    @retval int API number version

*/
jint getAPIVersion(JNIEnv *env){
    jclass versionClass = (*env)->FindClass(env, "android/os/Build$VERSION");
    jfieldID sdkIntFieldID = (*env)->GetStaticFieldID(env, versionClass, "SDK_INT", "I");
    jint sdkInt = (*env)->GetStaticIntField(env, versionClass, sdkIntFieldID);
    arthooklog("API version: %d \n", sdkInt);
    return sdkInt;

}

int isLollipop(JNIEnv *env)
{
    jint res = getAPIVersion(env);
    if(res > 19 )
        return 1;
    else return 0;
}

char* parseSignature(char* sig){
    char* p = sig;
    char* result = calloc(strlen(sig)+1, sizeof(char));
    char* copy = calloc(strlen(sig)+1, sizeof(char));
    result[strlen(sig)+1] = 0x0;
    strcpy(copy, sig);
    char* token = strtok(copy, "( )");
    char* obj = strtok(token, ";");
    while( *p ){
        size_t counter = 0;
        arthooklog("%s, porcodio1 vale: %c \n", __PRETTY_FUNCTION__, p[counter]);
        if(*p == ')')
            break;
        if(p[counter] == 'L'){
            strcat(result, "L|");
            char* res = strchr(p, ';');
            p = res;
            counter = (res - sig + 1);
        }
        if(p[counter] == 'I'){
            strcat(result, "I|");
        }
        if(p[counter] == '['){
            *p++;
            if(p[counter] == 'B'){
                strcat(result, "[B|");
            }
        }
        *p++;
    }
    return result;
}
void breakMe()
{
    return;
}

void set_pointer(unsigned int *s, unsigned int d){
    *s = d;
}

jobject createInstanceFromJClsName(JNIEnv* env, jstring jstr)
{
    const char * clsname;
    clsname = (*env)->GetStringUTFChars(env,jstr , NULL ) ;
    return createInstanceFromClsName(env, (char*) clsname);

}
char* getCharFromJstring(JNIEnv* env, jstring jstr)
{
    jsize len = (*env)->GetStringUTFLength(env, jstr);
    char* res = (char*) calloc(len+1,1);
    (*env)->GetStringUTFRegion(env, jstr, 0, len, res);
    //(*env)->DeleteLocalRef(env, jstr);
    arthooklog("getcharfromj, res: %s \n", res);
    return res;
}
jobject createInstanceFromClsName(JNIEnv* env, char* clsname)
{
    arthooklog("testjni str, search: %s \n", clsname);
    jclass target = (*env)->FindClass(env,clsname);
    if(!target)
        return (jobject)-1;
    jmethodID c = (*env)->GetMethodID(env,target, "<init>", "()V");
    jobject targetcls = (*env)->NewObject(env,target, c, NULL);
    arthooklog("ok\n");
    return targetcls;
}

/*
   unsigned int revgadget;
   revgadget =  ((gadget>>24)&0xff) | // move byte 3 to byte 0
   ((gadget<<8)&0xff0000) | // move byte 1 to byte 2
   ((gadget>>8)&0xff00) | // move byte 2 to byte 1
   ((gadget<<24)&0xff000000); // byte 0 to byte 3
   LOGI("reverse : 0x%x\n ", revgadget);
   */
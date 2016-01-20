#include <stdio.h>
#include "utils.h"


/*
    @name checkAPIVersion
    @brief return the API version number
    @retval int API number version

*/
jint getAPIVersion(){
    LOGG("%s CALLED!!!\n", __PRETTY_FUNCTION__);
    JNIEnv *env = get_global_jnienv();
    if(env == NULL){
        LOGG("ERROR %s getting jni\n", __PRETTY_FUNCTION__);
        return 1;
    }
    jclass versionClass = (*env)->FindClass(env, "android/os/Build$VERSION");
    jfieldID sdkIntFieldID = (*env)->GetStaticFieldID(env, versionClass, "SDK_INT", "I");
    jint sdkInt = (*env)->GetStaticIntField(env, versionClass, sdkIntFieldID);
    arthooklog("API version: %d \n", sdkInt);
    return sdkInt;

}

int isLollipop()
{
    jint res = getAPIVersion();
    if(res > 19 )
        return 1;
    else return 0;
}


static void _helper(JNIEnv* env, char *src, char* result, bool array, jvalue* args, int counter, int index, jobjectArray joa) {
    if (!src) return;
    arthooklog("src vale: %c, result: %s \n", *src, result);
    switch (*src) {
        case ')':
            break;
        case 'B':
            if (array) {
                strcat(result, "[B|");
                arthooklog("parser sto chiamanto getbytearray su %c con index = %d\n", *src, index);
                args[counter].l = callGetByteArray(env, joa, (jint) index);
                counter++;index++;
            }
            else {
                strcat(result, "B|");
            }
            *src++;
            return _helper(env, src, result, 0, args, counter, index,joa);
            break;
        case 'D':
            if (array) {
                strcat(result, "[D|");
            }
            else {
                strcat(result, "D|");
                args[counter].d = callGetDouble(env, joa, (jint) index);
                counter++;index++;
            }
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
        case 'Z':
            if (array)
                strcat(result, "[Z|");
            else {
                strcat(result, "Z|");
                args[counter].z = callGetBoolean(env, joa, (jint) index);
                counter++;index++;
            }
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
        case 'J':
            if (array)
                strcat(result, "[J|");
            else
                strcat(result, "J|");
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
        case 'I':
            if (array)
                strcat(result, "[I|");
            else {
                strcat(result, "I|");
                arthooklog("parser sto chiamanto getint su %c con index = %d \n", *src, index);
                args[counter].i = callGetInt(env, joa, (jint) index);
                counter++;index++;
            }
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
        case '[':
            *src++;
            return _helper(env,src, result, 1, args, counter, index,joa);
            break;
        case 'L':
            if (array) {
                strcat(result, "[L|");
                args[counter].l = callGetObj(env, joa, (jint) index) ;
                index++; counter++;
            }
            else {
                strcat(result, "L|");
                args[counter].l = callGetObj(env, joa, (jint) index);
                counter++;index++;
            }
            //search end obj
            char *res = strchr(src, ';');
            //goto end
            src = res;
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
        default:
            *src++;
            return _helper(env,src, result, 0, args, counter, index,joa);
            break;
    }
}
//static void _helper(JNIEnv* env, char *src, char* result, bool array,
// jvalue* args, int counter, int index, jobjectArray joa) {

char* parseSignature(JNIEnv* env, char* sig, jvalue* args, jobjectArray joa, int counter){
    char* p = sig;
    char* result = calloc(strlen(sig)+1, sizeof(char));
    result[strlen(sig)+1] = 0x0;
    _helper(env, p,result,0,args, counter, 0, joa);
    free(result);
}

jobject parseReturnType(JNIEnv* env, char *sig, jobject thiz, jclass c, jmethodID mid, jvalue* args)
{
    arthooklog("%s \n", __PRETTY_FUNCTION__);
    arthooklog("%s sig: %s \n", __PRETTY_FUNCTION__, sig);
    char* p = strchr(sig, ')');
    arthooklog("%s sig: %s \n", __PRETTY_FUNCTION__, p);
    p++;
    arthooklog("%s switch on %c \n", __PRETTY_FUNCTION__, *p);
    switch(*p){
        case 'V':
            callOriginalVoidMethod(env, thiz, c, mid, args);
            return NULL;
            break;
        case 'L':
            return callOriginalObjectMethod(env, thiz, c, mid, args);
            break;
        default:
            break;
    }
}
/*
 *
 * Signature : (Ljava/lang/Class;IBZDJ)V

char* parseSignature(char* sig){
    char* p = sig;
    int i = 0;
    char* result = calloc(strlen(sig)+1, sizeof(char));
    //char* copy = calloc(strlen(sig)+1, sizeof(char));
    result[strlen(sig)+1] = 0x0;
    //strcpy(copy, sig);
    //get arguments
    //char* token = strtok(copy, "( )");
    //get object type
    //char* obj = strtok(token, ";");
    while( *p ){
        size_t counter = 0;
        arthooklog("%s, porcodio1 vale: %c \n", __PRETTY_FUNCTION__, p[counter]);
        if(*p == ')')
            break;
        if(p[counter] == 'L'){
            strcat(result, "L|");
            //search end obj
            char* res = strchr(p, ';');
            //goto end
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
 */
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
//return 1 if 'searchme' is founded in the output of 'command'
//0 altrimenti
char* _getprop(char* command, char* searchme){
    char buffer[128];
    FILE* fp = popen(command, "r");
    if(fp == NULL){
        LOGG("ERROR getprop\n");
        return 0;
    }
    while( !feof(fp) ){
        if( fgets(buffer, 128, fp) != NULL){
            if(strstr(buffer, searchme) != NULL) return 1;
        }else{
            pclose(fp);
            return 0;
        }
    }
    pclose(fp);
    return 0;
}
/*
   unsigned int revgadget;
   revgadget =  ((gadget>>24)&0xff) | // move byte 3 to byte 0
   ((gadget<<8)&0xff0000) | // move byte 1 to byte 2
   ((gadget>>8)&0xff00) | // move byte 2 to byte 1
   ((gadget<<24)&0xff000000); // byte 0 to byte 3
   LOGI("reverse : 0x%x\n ", revgadget);
   */
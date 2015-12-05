/*
 * Helper functions for placing hooks
 *
 */

#include "arthook_helper.h"
/*
   wrapper function to hijack the vtable's pointer
*/
static int set_hook(JNIEnv *env, arthook_t *h)
{
    unsigned int* res;
    arthooklog("set_hook\n");
    if(h->isSingleton){
        // search the pointer inside the vtable
        res = searchInMemoryVtable( (unsigned int) h->original_cls, (unsigned int) h->original_meth_ID, isLollipop(env), false);
    }
    else{
        res = searchInMemoryVtable( (unsigned int) h->original_meth_ID, (unsigned int) h->original_meth_ID, isLollipop(env), false);
    }
    if(res == 0) {
        LOGG("search returned 0\n");
        return 0;
    }
    arthooklog("set_pointer 0x%08x with 0x%08x \n", (unsigned int) h->original_meth_ID, (unsigned int) h->hook_meth_ID);
    // change the pointer in the vtable
    set_pointer(res, (unsigned int ) h->hook_meth_ID);
/*    
       res = searchInMemoryVmeths( (unsigned int) h->original_meth_ID, (unsigned int) h->original_meth_ID, isLollipop(env));
       if(res == 0) return 0;
       LOGI("trovato SECONDO GIRO pointer at 0x%x \n",  res);
       set_pointer(res, (unsigned int ) h->hook_meth_ID); //TODO: set_vtable_pointer
*/    
    return 1;
}

/*
   @create_hook:

   This function create an arthook_t which represent the hook on the method "mname" of the "clsname"
   inside the "target" object.

   @param clsname target class name
   @param mname traget method name
   @param msig target method signature

   @return arthook_t the created arthook_t

*/
arthook_t* create_hook(JNIEnv *env, char *clsname, const char* mname,const  char* msig, jclass hook_cls, jmethodID hookm)
{
    arthooklog("------------------------------------------------------------------------------\n\n");
    arthooklog("%s mname: %s , msig: %s \n ", __PRETTY_FUNCTION__, mname, msig);
  
    arthook_t *tmp = NULL;
    jclass target = NULL;
    jclass gTarget = NULL;
    jmethodID target_meth_ID = NULL;

    tmp = (arthook_t*) malloc(sizeof(struct arthook_t));
    if(!tmp){
        LOGI("error malloc!\n");
        return NULL;           
    }
    //create unique hook hash
    strncpy(tmp->clsname, clsname, sizeof(tmp->clsname));
    strncpy(tmp->mname, mname, sizeof(tmp->mname));
    strncpy(tmp->msig, msig, sizeof(tmp->msig));
    memset(tmp->key, 0, sizeof(tmp->key));
    strcat(tmp->key,clsname);
    strcat(tmp->key,mname);
    strcat(tmp->key,msig);



    // TODO: addding of this hook in an automate way
    // SmsManager uses a singleton which in create at loading time
    if(strstr(tmp->key, "sendTextMessage")) {
        jclass cls = (*env)->FindClass(env, clsname);
        jmethodID getd = (*env)->GetStaticMethodID(env,cls, "getDefault", "()Landroid/telephony/SmsManager;");
        jobject res = (*env)->CallStaticObjectMethod(env, cls, getd);
        jfieldID field = (*env)->GetStaticFieldID(env, cls, "sInstance", "Landroid/telephony/SmsManager;");
        jobject o =  (*env)->GetStaticObjectField(env, cls, field);
        arthooklog("singleton : %p \n", o);
        arthooklog("SmsManager obj : %p\n", res);
        jclass target =(*env)->GetObjectClass(env, o);
        jmethodID mid = (*env)->GetMethodID(env,target, mname, msig);
        arthooklog("singleton mid: %p \n", mid);
        target_meth_ID = (*env)->GetMethodID(env, target, mname, msig);
        arthooklog("target mid: %p \n", target_meth_ID);
        gTarget = field;
        tmp->isSingleton = false;
    }
    else{
        // find the target class using JNI
        target = (*env)->FindClass(env, clsname);
        // make it a global ref for future access
        gTarget = (jclass) (*env)->NewGlobalRef(env, target);
        // get mid of the target method to hook
        target_meth_ID = (*env)->GetMethodID(env, target, mname, msig);
        tmp->isSingleton = false;
        
    }    
    // populate the arthook_t
    tmp->hook_meth_ID = hookm;
    tmp->original_meth_ID = target_meth_ID;
    tmp->original_obj = NULL;
    tmp->original_cls = gTarget;
    tmp->hook_cls = hook_cls;
    // set the hook in memory
    set_hook(env, tmp);
    arthooklog("------------------------------------------------------------------------------\n");
    return tmp;
}

/*
 * @get_artmethod_from_reflection:
 *
 * Called when an call to native MethodInvoke function is trapped by the framework.
 * This function retrive from the memory the called ArtMethod which is compared with
 * the hooked methods hashdict.
 *
 * @return the mid or NULL
 */

static void* get_artmethod_from_reflection(JNIEnv* env, jobject javaMethod){
    arthooklog("called %s with %p \n", __PRETTY_FUNCTION__, javaMethod);
    int lol = isLollipop(env);
    if(lol){
        arthooklog("ricevuto mid: %p \n", javaMethod);
        unsigned int* real = ((*(unsigned int*)javaMethod) + 0xc);
        if(!real)
            return NULL;
        arthooklog("chiamato metodo: %p = %x con la reflection \n", real, *real);
        return real;
    }
    jclass c = (*env)->FindClass(env, "java/lang/reflect/AbstractMethod");
    //jclass c1 = (*env)->FindClass(env, "java/lang/reflect/Method");
    jfieldID fid = (*env)->GetFieldID(env, c, "artMethod", "Ljava/lang/reflect/ArtMethod;");
    unsigned char* myfid = (unsigned char*) fid;
    unsigned char* off = (unsigned char*) (myfid + 0x14); //memory offset
    unsigned int* myclazz = javaMethod;
    unsigned int* mymid = (unsigned int*) (*myclazz + *off);
    if(mymid) {
        arthooklog("%s,founded mid: %p \n", __PRETTY_FUNCTION__,mymid);
        return mymid;
    }
    else {
        LOGG("%s, Error!!\n", __PRETTY_FUNCTION__);
        return NULL;
    }
}

/*
 * @hh_check_javareflection_call:
 *
 * Check if the trapped call (made using Java reflection) is to a hooked method
 *
 * @javMethod invoked java method
 * @javaReceiver this object
 * @return arthook_t or NULL
 *
 */
void* hh_check_javareflection_call(JNIEnv *env, jobject javaMethod, jobject javaReceiver)
{
    unsigned int* mymid = get_artmethod_from_reflection(env,javaMethod);
    if(! mymid)
        return NULL;
    int res = is_method_in_hashtable(mymid);
    void* ret = NULL;
    if(res){
        arthook_t* tmp = (arthook_t*) get_method_from_hashtable(mymid);
        return tmp;
    }
    return NULL;
}

/*
 * @tryToUnbox
 *
 * Unbox arguments of a Java reflection call
 *
 */
static jvalue* tryToUnbox(JNIEnv* env, arthook_t* hook, unsigned int* javaArgs,jobject thiz, bool call_patchmeth)
{
    arthooklog("called %s with : %p \n", __PRETTY_FUNCTION__, javaArgs);
    int * p = (int*) *javaArgs + 0x2;
    jobjectArray joa = (jobjectArray) javaArgs;
    int num = (*env)->GetArrayLength(env, joa);
    arthooklog("array con numero elementi: %d !!!! \n", num);
    jobject tmp;
    int counter = 0;
    int index = 0;
    if(call_patchmeth)
        num += 1;

    jvalue* args = calloc(num, sizeof(jvalue));
    arthooklog("pd signature: %s\n", hook->msig);
    char* res = parseSignature(hook->msig);
    arthooklog("after parse signature: %s\n", res);
    char* tok = strtok(res, "|");
    jstring s;
    jobject o;
    jint i;
    jdouble d;
    jfloat f;
    jboolean z;
    jlong j;
    jbyteArray jba;

    if(call_patchmeth) {
        args[0].l = thiz;
        counter++;
    }
    while(tok != NULL){
        arthooklog("trovato : %s\n", tok);
        if(*tok == 'L'){
            o = callGetObj(env, joa, (jint) index) ;
            arthooklog("Object %p  = %x, counter = %d\n", o, (unsigned int) o, counter);
            args[counter].l = (*env)->NewGlobalRef(env, o);
            counter++;
            index++;
        }
        else if(*tok == 'I'){
            arthooklog("parser sto chiamanto getint su %c\n", *tok);
            i = callGetInt(env, joa, (jint) index);
            arthooklog("int: %d\n", i);
            args[counter].i = i;
            counter++;
            index++;
        }
        else if(*tok == 'D'){
            d = callGetDouble(env, joa, (jint) index);
            arthooklog("double: %d\n", i);
            args[counter].d = d;
            counter++;
            index++;
        }
        else if(*tok == 'J'){
            j = callGetLong(env, joa, (jint) index);
            arthooklog("trovato long: %d\n", i);
            args[counter].j = j;
            counter++;
            index++;
        }
        else if(*tok == 'Z'){
            z = callGetBoolean(env, joa, (jint) index);
            arthooklog("result int: %d\n", i);
            args[counter].z = z;
            counter++;
            index++;
        }
        else if(*tok == '['){
            arthooklog("parser sto chiamando NULL su %c\n", *tok);
            //if(*tok == 'B') {
                arthooklog("parser sto chiamanto getbytearray su %c\n", *tok);
                jba = callGetByteArray(env, joa, (jint) index);
                args[counter].l = jba;
                counter++;
                index++;
            //}
        }
        tok = strtok(NULL, "|");
    }
    return args;
}

static void* searchInMemory(unsigned int *start, unsigned int target, unsigned int* len){
    unsigned char *g2 = (unsigned char*) &target;
    unsigned char *p = (unsigned char*)  *start;
    int i = 0;
    //searching the method reference inside the class's vtable
    while(1){
        //vtable bound check
        if(i > (*len * 4) + 4  )
            return 0;
        arthooklog("check p = %p and target 0x%08x \n",p, target);
        if(! memcmp(p, g2, 4 )){
            unsigned int* found_at = (unsigned int*) (*start + i) ;
            arthooklog("target VTABLE: 0x%08x at %p. start = %x, bytes = %d \n", target, found_at, *start, i);
            return found_at;
        }
        p += 4; i += 4;
    }
}

/*
 * @callOriginalReflectionmethod
 *
 * used to call the original method of a java reflection call.
 *
 * @return a global ref to the returned value (if any)
 */
void* callOriginalReflectedMethod(JNIEnv* env, jobject thiz, arthook_t* hook, jobject javaArgs){
    arthooklog("%s, target hook clsname: %s \n", __PRETTY_FUNCTION__, hook->clsname);
    jvalue* args;
    jobject res = NULL;
    if(javaArgs)
         args =  tryToUnbox( env, hook, (unsigned int*) javaArgs, thiz, false);
    else{
        jclass t = (*env)->GetObjectClass(env, thiz);
        res = (*env)->CallNonvirtualObjectMethod(env, thiz, t, hook->original_meth_ID);
    }
    if(args){
        jclass t = (*env)->GetObjectClass(env, thiz);
        if(strstr(hook->key, "sendTextMessage")){
            (*env)->CallNonvirtualVoidMethodA(env, thiz, t, hook->original_meth_ID,args);
        }
        else
            res = (*env)->CallNonvirtualObjectMethodA(env, thiz, t, hook->original_meth_ID,args);
        if(res != NULL)
            return res;
           //return (*env)->NewGlobalRef(env, res);
        else return NULL;
    }
}

/*
 * @call_patch_method
 *
 * Call the patch method associated to the arthook_t h
 *
 */
jobject call_patch_method(JNIEnv* env, arthook_t* h, jobject thiz, jobject javaArgs){
    jvalue* args;
    jobject res = NULL;
    if(javaArgs){
        args = tryToUnbox(env, h, (unsigned int*) javaArgs, thiz, true);
    }
    else{
        res = (*env)->CallStaticObjectMethod(env, h->hook_cls, h->hook_meth_ID, thiz);
        return res;
        //return (*env)->NewGlobalRef(env, res);
    }
    if(args){
        jclass t = (*env)->GetObjectClass(env, thiz);
        res = (*env)->CallStaticObjectMethodA(env, h->hook_cls, h->hook_meth_ID, args);
        return res;
        //return (*env)->NewGlobalRef(env, res);
    }
    else{
        return NULL;
    }
}
static unsigned int* searchInMemoryVtable_kk(unsigned int start, int target, bool isClass) {
    unsigned int* pClazz;
    unsigned int* pAccess_flags;
    unsigned int* vtable;
    unsigned int* vmethods_len;

    pClazz = (unsigned int *) (start + CLAZZ_OFF_);
    pAccess_flags = (unsigned int *) (start + KK_ACCESS_FLAG_OFF);
    vtable =  (unsigned int*) ((*pClazz) + KK_VTABLE_OFF);
    vmethods_len = (unsigned int*) ((*vtable) + VMETHODS_LEN_OFF_);
    arthooklog("pclazz: %p = %x\n", pClazz, *pClazz);
    arthooklog("vmethods len: %zd \n", *vmethods_len);
    arthooklog("clazz: %p, * = 0x%08x [] vtable=%p, * = 0x%08x \n", pClazz , *pClazz, vtable, *vtable);
    return searchInMemory(vtable, target, vmethods_len);
}
static unsigned int* searchInMemoryVtable_all(unsigned int start, int target) {
    unsigned int* pClazz;
    unsigned int* pAccess_flags;
    unsigned int* mid_handler;
    int* mid_index;
    int midvtable;

    //getting declaring_class_
    pClazz = (start + CLAZZ_OFF_);
    arthooklog("pclazz vale: %p = %08x \n", pClazz, *pClazz);
    mid_index = (start + LOLLIPOP_MID_INDEX_OFF );
    arthooklog("mid_index vale: %d \n", *mid_index);
    arthooklog("ora calcolo %08x  + %d * 4 \n", *pClazz, *mid_index);
    midvtable = ((int)(*pClazz) + (*mid_index * 4));
    arthooklog("midvtable vale: %d \n", midvtable);
    mid_handler = midvtable + 0x170;
    arthooklog("handler2: %p = %08x \n", mid_handler, *mid_handler);

    arthooklog("%s clazz: %p, * = 0x%08x , midindex = %08x [] handler=%p, * = 0x%08x \n", __PRETTY_FUNCTION__,
               pClazz , *pClazz, *mid_index,
               mid_handler, *mid_handler);
    return mid_handler;
}
/*
   Search a pointer in  the process' virtual memory, starting from address @start.

*/
static unsigned int* searchInMemoryVtable(unsigned int start, int gadget, int lollipop, bool isClass){
    arthooklog("%s start : 0x%08x, gadget 0x%08x, lollipop = %d \n", __PRETTY_FUNCTION__, start, gadget, lollipop);
    // > 4.4.4 offsets
    if(lollipop){
        return searchInMemoryVtable_all(start,gadget);
    }
    //kitkat 4.4.4 with art offsets
    else{
        return searchInMemoryVtable_kk(start,gadget,isClass);
    }
}

// TODO: to be completed
/*
static unsigned int* searchInMemoryVmeths(unsigned int start, int gadget, int lollipop){
    //LOGI("start VMETHODS: 0x%08x, gadget 0x%08x, lollipop = %d \n", start, gadget, lollipop);
    unsigned int i = 0;
    unsigned int *pClazz;
    unsigned int* pAccess_flags;
    unsigned int *vtable;
    unsigned int *vmethods;
    // > 4.4.4 offsets
    if(lollipop){
        pClazz = (unsigned int*) (start + LOLLIPOP_CLAZZ_OFF);
        vtable =  (unsigned int*) ((*pClazz) +  LOLLIPOP_VMETHODS_OFF);
        vmethods =  (unsigned int*) ((*pClazz) +  LOLLIPOP_VMETHODS_OFF);
        //LOGI("LOLLIPOP clazz: 0x%08x, * = 0x%08x -> vtable=0x%08x, * = 0x%08x, vmethods = 0x%08x, * = 0x%08x \n", pClazz , *pClazz, vtable, *vtable, vmethods, *vmethods );
    }
    //kitkat 4.4.4 with art offsets
    else{
        pClazz = (unsigned int*) (start + CLAZZ_OFF);
        pAccess_flags = (unsigned int*) (start + ACCESS_FLAG_OFF);
        vtable =  (unsigned int*) ((*pClazz) + VTABLE_OFF);
        vmethods = (unsigned int*) ((*pClazz) + ITABLE_OFF);
        if(! (*vmethods) ) return 0;
    }
    breakMe();
    unsigned char *g2 = (unsigned char*) &gadget; 
    unsigned char *p = (unsigned char*)  *vmethods;
    arthooklog("INIZIO RICERCA\n");
    //searching the method reference inside the class's vtable
    while(1){
        //LOGI("check p 0x%08x and gadget 0x%08x \n",p, gadget);
        if(! memcmp(p, g2, 4 )){
            unsigned int* found_at = (unsigned int*) (*vmethods + i) ;
            //LOGI("target: 0x%08x at 0x%08x. vmeths = %x, i = %d \n", gadget, found_at, *vmethods, i);
            //changed = 1;

            return found_at;
        }
        p += 4; i += 4;
    }
    return 0;
}

*/

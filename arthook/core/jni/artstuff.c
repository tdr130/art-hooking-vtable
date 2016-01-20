/*
 * Exploiting libart exported functions
 */

#include "artstuff.h"

/* wrapper per dlsym() */
static void* mydlsym(void *hand, const char *name)
{
	void* ret = dlsym(hand, name);
	arthooklog("%s = %p\n", name, ret);
	return ret;
}

void inline test_asm(void* this, unsigned int flags, void* funz){
    __asm __volatile (
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            "blx r2\n"
            :
            :   "r" (this), "r" (flags), "r" (funz)
            :   "r0", "r1", "r2"
    );
}
void call_test_asm(struct artstuff_t *d){
    arthooklog("calling dumpallclasses with: %x\n", d->art_dumpallclasses_fnPtr);
    test_asm(d->art_runtime_instance,1,d->art_dumpallclasses_fnPtr);
}
int resolve_symbols(struct artstuff_t *d){
	d->art_hand = dlopen("libart.so", RTLD_NOW);
	arthooklog("art_hand = 0x%08x \n", (unsigned int) d->art_hand);
	if(d->art_hand != NULL){
		d->JNI_GetCreatedJavaVMs_fnPtr = mydlsym(d->art_hand, "JNI_GetCreatedJavaVMs");
        d->art_th_currentFromGdb_fnPtr = mydlsym(d->art_hand, "_ZN3art6Thread14CurrentFromGdbEv");
        d->art_dbg_SuspendVM_fnPtr = mydlsym(d->art_hand, "_ZN3art3Dbg9SuspendVMEv");
        d->art_dbg_ResumeVM_fnPtr = mydlsym(d->art_hand, "_ZN3art3Dbg8ResumeVMEv");
        d->art_dbg_SuspendSelf_fnPtr = mydlsym(d->art_hand, "_ZN3art3Dbg11SuspendSelfEv");
        d->art_thlist_resumeAll_fnPtr = mydlsym(d->art_hand, "_ZN3art10ThreadList9ResumeAllEv");
		d->art_runtime_instance = mydlsym(d->art_hand,"_ZN3art7Runtime9instance_E");
        d->art_dumpallclasses_fnPtr = mydlsym(d->art_hand,"_ZN3art11ClassLinker14DumpAllClassesEi");
        //d->art_printloadedclasses_fnPtr = mydlsym(d->art_hand,"_ZN3artL26VMDebug_printLoadedClassesEP7_JNIEnvP7_jclassi");
	}
    else {
        LOGG("cannot dlopen libart.so");
        LOGG("ERROR are you using the ART runtime? \n");
        return 1;
    }
	return 0;
}

void _GetCreatedJavaVMs(struct artstuff_t* d, void** vms, jsize size, jsize* vm_count){
	arthooklog("dentro getcreatedjavavms \n");
	jint res = 3;
	arthooklog("chiamo: 0x%08x \n ", (unsigned int) d->JNI_GetCreatedJavaVMs_fnPtr);
	res = d->JNI_GetCreatedJavaVMs_fnPtr(vms, size, vm_count);
	arthooklog("res = %d\n", res);
	if(res != JNI_OK){
		arthooklog("error!!!!\n");
	}
}

void _suspendAllForDbg(struct artstuff_t *d)
{
    arthooklog("suspend vm: %x \n", (unsigned int) d->art_dbg_SuspendVM_fnPtr );
    //d->art_thlist_suspendAllForDbg_fnPtr();
    //d->art_th_DumpFromDbg_fnPtr();
    //d->art_dbg_SuspendVM_fnPtr();
    //d->art_dbg_SuspendSelf_fnPtr();
    //d->art_thlist_resumeAll_fnPtr();
}



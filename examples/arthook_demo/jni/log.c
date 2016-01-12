//
// Created by vaioco on 12/01/16.
//

#include "log.h"

static char logfile[] = "/data/local/tmp/adbi.log";
extern void (*log_function)(char *logmsg);


#undef log
#define log(...) \
        {FILE *fp = fopen(logfile, "a+"); if (fp) {\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}}

char artlogfile[]  = "/data/local/tmp/arthook.log";
/*
 *  log function to pass to the hooking library to implement central loggin
 *
 *  see: set_logfunction() in base.h
 */
void my_log(char *msg)
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
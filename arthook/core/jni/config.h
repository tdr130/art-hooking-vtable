#ifndef _CONFIGH_
#define _CONFIGH_

#define VERSION "0.1.0"
#define MYOPTDIR  "/data/local/tmp/dex/opt"

#include "json_parser/json.h"
#include <jni.h>

static char* command_runtime = "getprop ";
static char* command_api = "";
static char* command_version = "";

typedef struct config_t{
    int version;
    int debug;
    char* fname;
    char* optdir;
    char* dexfile;
    int osversion;
    int (*parser)(struct config_t*);
} config_t;


int config_init_working_dir(struct config_t* c);
 void printJson (json_value *node, int indent, char* name);
static void process_value(json_value* value, int depth);
void* config_init(char* fname);

#endif
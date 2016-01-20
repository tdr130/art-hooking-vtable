#ifndef _CONFIGH_
#define _CONFIGH_

#define VERSION "0.1.0"
#define MYOPTDIR  "/data/local/tmp/dex/opt"

#include "json_parser/json.h"

static command_runtime = "getprop ";
static command_api = "";
static command_version = "";

typedef struct config_t{
    int version;
    char* fname;
    char* optdir;
    int osversion;
    int zygote;
    int (*parser)(struct config_t*);
} config_t;

 void printJson (json_value *node, int indent, char* name);
static void process_value(json_value* value, int depth);
void* config_init(char* fname, bool);

#endif
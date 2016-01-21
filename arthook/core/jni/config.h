#ifndef _CONFIGH_
#define _CONFIGH_

#define VERSION "0.1.0"
#define MYOPTDIR  "/data/local/tmp/dex/opt"
#include <stdbool.h>

#include "json_parser/json.h"

static char* command_runtime = "getprop";
static char* command_api = "";
static char* command_version = "";
static char* command_processIsZygote = "ps | grep zygote | tr -s \" \" | cut -d \" \" -f 2";

typedef struct config_t{
    int version;
    int debug;
    char* fname;
    char* optdir;
    char* dexfile;
    int osversion;
    int zygote;
    int (*parser)(struct config_t*);
} config_t;

typedef config_t* configT_ptr;

int config_init_working_dir(struct config_t* c);
void printJson (json_value *node, int indent, char* name);
static void process_value(json_value* value, int depth);
void* config_init(char* fname, bool);
configT_ptr getConfig();
char* _config_create_env();
#endif
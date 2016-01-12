//
// Created by vaioco on 12/01/16.
//

#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "config.h"
#include "arthook_manager.h"

static void print_depth_shift(int depth)
{
    int j;
    for (j=0; j < depth; j++) {
        arthooklog(" ");
    }
}


static void process_object(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        print_depth_shift(depth);
        arthooklog("object[%d].name = %s\n", x, value->u.object.values[x].name);
        process_value(value->u.object.values[x].value, depth+1);
    }
}

static void process_array(json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.array.length;
    arthooklog("array\n");
    for (x = 0; x < length; x++) {
        process_value(value->u.array.values[x], depth);
    }
}

static void process_value(json_value* value, int depth)
{
    int j;
    if (value == NULL) {
        return;
    }
    if (value->type != json_object) {
        print_depth_shift(depth);
    }
    switch (value->type) {
        case json_none:
            arthooklog("none\n");
            break;
        case json_object:
            process_object(value, depth+1);
            break;
        case json_array:
            process_array(value, depth+1);
            break;
        case json_integer:
            arthooklog("int: %10" PRId64 "\n", value->u.integer);
            break;
        case json_double:
            arthooklog("double: %f\n", value->u.dbl);
            break;
        case json_string:
            arthooklog("string: %s\n", value->u.string.ptr);
            break;
        case json_boolean:
            arthooklog("bool: %d\n", value->u.boolean);
            break;
    }
}

int parse_simply(struct config_t* c){
    struct stat file_status;
    int file_size;
    int fp;
    char* file_contents;
    json_char* json;
    json_value* node;

    if( stat(c->fname, &file_status) != 0 ){
        LOGG("ERROR stat on %s\n", c->fname);
        return 1;
    }

    file_size = file_status.st_size;
    file_contents = (char*) malloc(file_size);
    if( file_contents == NULL){
        LOGG("ERROR malloc \n");
        return 1;
    }
    fp = fopen(c->fname, "rt");
    if(fp == NULL){
        LOGG("ERROR fopen %s \n", c->fname);
        free(file_contents);
        return 1;
    }
    if ( fread(file_contents, file_size,1,fp) != 1){
        LOGG("ERROR fread\n");
        free(file_contents);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    json = (json_char*) file_contents;
    node = json_parse(json,file_size);
    if(node == NULL){
        LOGG("ERROR while parsing %s \n", c->fname);
        free(file_contents);
        fclose(fp);
        return 1;
    }
    process_value(node,0);
    json_value_free(node);
    free(file_contents);
    return 0;

}
static int  program_version(){
    return atoi(VERSION);
}

static int _check_runtime(){
    if(_getprop(command_runtime, "libart.so"))
        return 1;
    else return 0;
}
static char* _config_create_env(){
    char tmpdir[256];
    char ppid[8];

    arthooklog("dentro %s, starting %d ....\n", __PRETTY_FUNCTION__, getpid());

    sprintf(tmpdir,"%s/",MYOPTDIR);
    //zygote fix
    //entro dentro la cartella tmp_root e creo una mia dir
    if( chdir(MYOPTDIR) ){
        LOGG("ERROR chdir!!\n");
        return 0;
    }
    sprintf(ppid,"%d",getpid());
    strcat(tmpdir,ppid);
    arthooklog("creo working dir: %s \n", tmpdir);
    if( mkdir(tmpdir, 0777) != 0 ){
        LOGG("ERROR mkdir: %s \n", strerror(errno));
        return 0;
    }
    if( chdir(tmpdir) ){
        LOGG("ERROR chdir!!\n");
        return 0;
    }
    return tmpdir;
}
void* config_init(char* fname){
    char* work_dir;
    arthooklog("%s called with : %s len %d \n", __PRETTY_FUNCTION__, fname, strlen(fname) );
    struct config_t* c = (struct config_t*) calloc(1,sizeof(struct config_t));
    if( c == NULL){
        LOGG("ERROR malloc!!\n");
        return NULL;
    }
    c->fname = (char*) calloc(strlen(fname) + 1, sizeof(char));
    if( c->fname == NULL){
        LOGG("ERROR malloc!!\n");
        free(c);
        return NULL;
    }
    strcpy(c->fname,fname);
    c->parser = parse_simply;
    if( c->parser(c) != 0){
        LOGG("ERROR in json parser!!\n");
        //free(c);
        //return NULL;
    }

    c->version = program_version();
    int api = (int) getAPIVersion();
    if( api == 1){
        LOGG("ERROR GETTING API VERSION\n");
        free(c);
        return NULL;
    }
    c->osversion = api;
    if( _check_runtime() == 0 ){
        LOGG("ERROR you must set the ART as default runtime!! \n");
        free(c);
        return NULL;
    }
    work_dir = _config_create_env();
    if( work_dir == 0){
        LOGG("ERROR CREATE ENV\n");
        free(c);
        return NULL;
    }
    arthooklog("working dir: %s , len = %d \n ", work_dir, strlen(work_dir));
    c->optdir = (char*) calloc(strlen(work_dir) + 1, sizeof(char));
    strncpy(c->optdir,work_dir,strlen(work_dir));
    c->optdir[strlen(work_dir)] = 0x0;
    return c;
}

void config_free(struct config_t* c){
    free(c);
}
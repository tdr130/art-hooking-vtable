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


static int parsing_hook_objects(json_value* value, int depth){
    int length,x,i,l;
    json_value* root = NULL;
    json_value* tmp = NULL;
    meth_hooks_p target;

    length = value->u.array.length;
    arthooklog("hooks objects num %d\n", length);
    for(x = 0; x < length; x++){
        target = (meth_hooks_p )malloc(sizeof(methods_to_hook_t));
        if( target == NULL){
            LOGG("%s ERROR malloc \n", __PRETTY_FUNCTION__);
            return 1;
        }
        tmp = value->u.array.values[x];
        l = tmp->u.object.length;
        for ( i = 0; i < l ;i++){
            arthooklog("name: %s  = %s \n", tmp->u.object.values[i].name,tmp->u.object.values[i].value->u.string.ptr );
        }
        createInfoTarget(target, tmp, l);
        addTargetToList(target);
    }
    targetListIterator(NULL);
    return 0;
}
static void start_parsing(json_value* value, config_t* c){
    json_value* root = NULL;

    if (value == NULL)
        return;
    arthooklog("root vale: %s\n", value->u.object.values[0].name);
    if( strcmp(value->u.object.values[0].name, "config") != 0){
        return;
    }
    root = value->u.object.values[0].value; //debug object
    arthooklog("debug vale: %s\n", root->u.object.values[0].name);
    int debug = root->u.object.values[0].value->u.integer;
    arthooklog("DEBUG: %d\n", debug);
    c->debug = debug;

    arthooklog("dexfile vale: %s\n", root->u.object.values[1].name);
    char* dexfile = root->u.object.values[1].value->u.string.ptr;
    arthooklog("dex target: %s\n", dexfile);
    c->dexfile = (char* )malloc(sizeof(char) * strlen(dexfile) + 1);
    strncpy(c->dexfile, dexfile, strlen(dexfile));
    c->dexfile[strlen(dexfile)] = 0x0;

    arthooklog("hooks vale: %s\n", root->u.object.values[2].name);
    root = root->u.object.values[2].value;
    parsing_hook_objects(root, 2);
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
    //process_value(node,0);
    start_parsing(node,c);
    json_value_free(node);
    free(file_contents);
    return 0;

}

static int  program_version(){
    return atoi(VERSION);
}

static int _check_runtime(){
    int res = (int) _runCommand(command_runtime, "libart.so");
    if(res)
        return 1;
    else return 0;
}
char* _config_create_env(){
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
void* config_init(char* fname, bool zygote){
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
    if( !zygote ) {
        c->zygote = 0;
        work_dir = _config_create_env();
        if (work_dir == 0) {
            LOGG("ERROR CREATE ENV\n");
            free(c);
            return NULL;
        }
        arthooklog("working dir: %s , len = %d \n ", work_dir, strlen(work_dir));
        c->optdir = (char*) calloc(strlen(work_dir) + 1, sizeof(char));
        strncpy(c->optdir,work_dir,strlen(work_dir));
        c->optdir[strlen(work_dir)] = 0x0;
    }else{
        c->zygote = 1;
    }

    return c;
}

void config_free(struct config_t* c){
    free(c);
}
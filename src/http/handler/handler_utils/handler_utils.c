#include "handler_utils.h"

static const mime_type_t mime_types[] = {
    { "html", "text/html; charset=utf-8" },
    { "css",  "text/css" },
    { "js",   "application/javascript" },
    { "png",  "image/png" },
    { "jpg",  "image/jpeg" },
    { "ico",  "image/x-icon" },
    { NULL,   NULL }
};


char *get_mime_type(char *path){
    char *ext = strrchr(path, '.');
    if(!ext){
        return "application/octet-stream";
    }
    ext++;
    for (int i = 0; mime_types[i].ext != NULL; i++){
        if(strcmp(ext, mime_types[i].ext) == 0){
            return mime_types[i].mime;
        }
    }
    return "application/octet-stream";
}
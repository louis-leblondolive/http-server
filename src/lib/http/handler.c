#include "handler.h"

static const mime_type mime_types[] = {
    { "html", "text/html; charset=utf-8" },
    { "css",  "text/css" },
    { "js",   "application/javascript" },
    { "png",  "image/png" },
    { "jpg",  "image/jpeg" },
    { "ico",  "image/x-icon" },
    { NULL,   NULL }
};
static const int mime_types_count = 7;

char *get_mime_type(char *path){
    char *ext = strrchr(path, '.');
    if(!ext){
        return "application/octet-stream";
    }
    ext ++;
    for (int i = 0; i < mime_types_count; i++){
        if(strcmp(ext, mime_types[i].ext) == 0){
            return mime_types[i].mime;
        }
    }
    return "application/octet-stream";
}


bool file_is_reachable(char *path){
    return true;
}


char *copy_file(FILE *stream){

    char cache;
    int len = 0;

    fseek(stream, 0, SEEK_SET);
    while(fscanf(stream, "%c", &cache) != EOF){
        len ++;
    }

    char *copy = (char*)malloc(sizeof(char) * len);

    fseek(stream, 0, SEEK_SET);
    int cursor = 0;
    while(fscanf(stream, "%c", &cache) != EOF){
        copy[cursor] = cache;
        cursor ++;
    }

    copy[cursor] = '\0';

    return copy;
}



int handle_error(response *serv_resp, int error_flag){

    reset_response(serv_resp);

    if (init_response_status(serv_resp, error_flag) != 0) return -1;

    char *reason = http_reason(error_flag);

    if (add_header(serv_resp, "Content-Type", "text/html") != 0) return -1;

    char body[128];
    snprintf(body, sizeof(body), "<h1>%d - %s</h1>", error_flag, reason);
    strcpy(serv_resp->body, body);

    if (init_response_content_length(serv_resp) != 0) return -1;

    return 0;
}


int handle_get(request *client_req, response *serv_resp){

    // get copy of file 
    if(!file_is_reachable(client_req->path)) return handle_error(serv_resp, 403); 

    if(strcmp(client_req->path, "/") == 0){
        strlcpy(client_req->path, "www/index.html", MAX_PATH_LEN);
    } else {
        char new_path[MAX_PATH_LEN];
        snprintf(new_path, MAX_PATH_LEN, "www/%s", client_req->path);
        strlcpy(client_req->path, new_path, MAX_PATH_LEN);
    }

    FILE *stream = fopen(client_req->path, "rb");

    if(stream == NULL) return handle_error(serv_resp, 404);

    char *copy = copy_file(stream);

    fclose(stream);

    // build response 
    int cache_res = 0;

    strlcpy(serv_resp->body, copy, MAX_BODY_LEN);
    free(copy);

    cache_res = init_response_status(serv_resp, 200);
    if(cache_res != 0) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Content-Type", get_mime_type(client_req->path));
    if(cache_res != 0) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Connection", "close");
    if(cache_res != 0) return handle_error(serv_resp, cache_res);

    cache_res = init_response_content_length(serv_resp);
    if(cache_res != 0) return handle_error(serv_resp, cache_res);

    return 0;
}
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




char *copy_file(FILE *stream){

    char cache;
    int len = 0;

    fseek(stream, 0, SEEK_SET);
    while(fscanf(stream, "%c", &cache) != EOF){
        len ++;
    }

    char *copy = (char*)malloc(sizeof(char) * len);
    if(!copy) return NULL;

    fseek(stream, 0, SEEK_SET);
    int cursor = 0;
    while(fscanf(stream, "%c", &cache) != EOF){
        copy[cursor] = cache;
        cursor ++;
    }

    copy[cursor] = '\0';

    return copy;
}



http_status handle_error(response *serv_resp, http_status err_status){

    reset_response(serv_resp);

    if (init_response_status(serv_resp, err_status) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    const http_reason_code *reason_code = get_http_reason(err_status);

    if (add_header(serv_resp, "Content-Type", "text/html") != HTTP_OK) return HTTP_INTERNAL_ERROR;

    char body[128];
    snprintf(body, sizeof(body), "<h1>%d - %s</h1>", reason_code->code, reason_code->reason);
    strcpy(serv_resp->body, body);

    if (init_response_content_length(serv_resp) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    return HTTP_OK;
}


http_status handle_get(request *client_req, response *serv_resp){

    // get copy of file 

    FILE *stream = fopen(client_req->path, "rb");

    if(stream == NULL) return handle_error(serv_resp, HTTP_NOT_FOUND);

    char *copy = copy_file(stream);
    if(!copy) return handle_error(serv_resp, HTTP_INTERNAL_ERROR);

    fclose(stream);

    // build response 
    http_status cache_res;

    strlcpy(serv_resp->body, copy, MAX_BODY_LEN);
    free(copy);

    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Content-Type", get_mime_type(client_req->path));
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Connection", "close");
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = init_response_content_length(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    return HTTP_OK;
}
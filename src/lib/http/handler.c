#include "handler.h"


// --- UTILS ---------------   

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


// --- HANDLE FUNCTIONS ---------------    


http_status handle_error(response *serv_resp, http_status err_status){

    reset_response(serv_resp);

    if (init_response_status(serv_resp, err_status) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    const http_reason_code *reason_code = get_http_reason(err_status);

    if (add_header(serv_resp, "Content-Type", "text/html") != HTTP_OK) return HTTP_INTERNAL_ERROR;

    if (add_header(serv_resp, "Connection", "close") != HTTP_OK) return HTTP_INTERNAL_ERROR;
    strlcpy(serv_resp->connection_type, "close", MAX_HEADER_VALUE_SIZE);

    if(err_status != HTTP_NO_CONTENT && err_status != HTTP_NOT_MODIFIED
        && reason_code->code / 100 != 1){
    
        char body[128];
        snprintf(body, sizeof(body), "<h1>%d - %s</h1>", reason_code->code, reason_code->reason);
        strcpy(serv_resp->body, body);

        serv_resp->body_len = strlen(serv_resp->body);
    }
    else {
        serv_resp->body_len = 0;
    }

    if (init_response_content_length(serv_resp) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    return HTTP_OK;
}


http_status handle_get(request *client_req, response *serv_resp, bool head_only){

    if(!head_only){

        // read and copy files
        FILE *stream = fopen(client_req->path, "rb");

        if(stream == NULL) return handle_error(serv_resp, HTTP_NOT_FOUND);

        char *copy = copy_file(stream);
        if(!copy) return handle_error(serv_resp, HTTP_INTERNAL_ERROR);

        fclose(stream);
        
        
        // add body to response 
        strlcpy(serv_resp->body, copy, MAX_BODY_LEN);
        free(copy);
    }

    // add headers
    http_status cache_res;

    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Content-Type", get_mime_type(client_req->path));
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    struct stat st;
    if(stat(client_req->path, &st) == -1){
        return handle_error(serv_resp, HTTP_NOT_FOUND);
    }
    serv_resp->body_len = st.st_size;
    
    char last_modified[128];
    struct tm *tm_info = gmtime(&st.st_mtime);
    strftime(last_modified, sizeof(last_modified), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
    cache_res = add_header(serv_resp, "Last-Modified", last_modified);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    if(strcasecmp(client_req->connection_type, "keep-alive") == 0){
        strlcpy(serv_resp->connection_type, "keep-alive", MAX_HEADER_VALUE_SIZE);
    } else {
        strlcpy(serv_resp->connection_type, "close", MAX_HEADER_VALUE_SIZE);
    }
    cache_res = add_header(serv_resp, "Connection", serv_resp->connection_type);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = init_response_content_length(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    return HTTP_OK;
}


http_status handle_options(response *serv_resp){

    http_status cache_res;

    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Allow", ALLOWED_METHODS);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    serv_resp->body_len = 0;
    cache_res = init_response_content_length(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(serv_resp, cache_res);

    return HTTP_OK;
}
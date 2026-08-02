#include "http_request.h"



request_t *init_http_request(void){
    request_t *req = (request_t *)malloc(sizeof(request_t));
    if(!req) return NULL;

    req->body_len = 0;
    req->header_count = 0;
    
    req->method[0] = '\0';
    req->path[0] = '\0';
    req->version[0] = '\0';
    req->body[0] = '\0';
    req->connection_type[0] = '\0';

    for (size_t i = 0; i < MAX_HEADER_NB; i++){
        req->headers[i].key[0] = '\0';
        req->headers[i].value[0] = '\0';
    }

    req->header_count = 0;
    req->body_len = 0;

    return req;
}


void free_http_request(request_t *req){
    free(req);    
}
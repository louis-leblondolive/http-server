#include "http_request.h"



request_t *init_http_request(void){
    request_t *req = (request_t *)malloc(sizeof(request_t));
    if(!req) return NULL;

    req->body_len = 0;
    req->header_count = 0;

    return req;
}


void free_http_request(request_t *req){
    free(req);    
}
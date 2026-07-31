#ifndef STRUCTURES
#define STRUCTURES

#include <sys/socket.h>

#include "config.h"
#include "http_request.h"

typedef enum http_status {
    HTTP_OK,
    HTTP_CREATED, 
    HTTP_NO_CONTENT,

    HTTP_NOT_MODIFIED,

    HTTP_BAD_REQUEST,
    HTTP_UNAUTHORIZED,
    HTTP_FORBIDDEN,
    HTTP_METHOD_NOT_ALLOWED,
    HTTP_NOT_FOUND,
    HTTP_REQUEST_TIMEOUT,
    HTTP_REQUEST_ENTITY_TOO_LARGE,
    HTTP_URI_TOO_LONG, 
    HTTP_EXPECTATION_FAILED,
    HTTP_TEAPOT, 
    HTTP_HEADER_TOO_LARGE, 

    HTTP_INTERNAL_ERROR,
    HTTP_NOT_IMPLEMENTED,
    HTTP_BAD_GATEWAY,
    HTTP_VERSION_NOT_SUPPORTED

} http_status_e;



typedef struct response_head_s {
    char version[MAX_VERSION_LEN];
    char code[MAX_CODE_LEN];
    char reason[MAX_REASON_LEN];

    header_t headers[MAX_HEADER_NB];
    int header_count;

    size_t content_len;

} response_head_t;

#endif
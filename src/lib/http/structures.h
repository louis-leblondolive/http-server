#ifndef STRUCTURES
#define STRUCTURES

#include "config.h"


typedef enum Http_status {
    HTTP_OK,
    HTTP_CREATED, 
    HTTP_NO_CONTENT,

    HTTP_BAD_REQUEST,
    HTTP_UNAUTHORIZED,
    HTTP_FORBIDDEN,
    HTTP_NOT_FOUND,
    HTTP_REQUEST_ENTITY_TOO_LARGE,
    HTTP_URI_TOO_LONG, 
    HTTP_TEAPOT, 
    HTTP_HEADER_TOO_LARGE, 

    HTTP_INTERNAL_ERROR,
    HTTP_NOT_IMPLEMENTED,
    HTTP_VERSION_NOT_SUPPORTED

} http_status;

typedef struct Header {
    char key[MAX_HEADER_KEY_SIZE];
    char value[MAX_HEADER_VALUE_SIZE];
} header ;

typedef struct Request {
    char method[MAX_METHOD_LEN];
    char path[MAX_PATH_LEN];
    char version[MAX_VERSION_LEN];
    header headers[MAX_HEADER_NB];
    int header_count;
    char body[MAX_BODY_LEN];
} request;

typedef struct Response {
    char version[MAX_VERSION_LEN];
    char code[MAX_CODE_LEN];
    char reason[MAX_REASON_LEN];
    header headers[MAX_HEADER_NB];
    int header_count;
    char body[MAX_BODY_LEN];
} response;

#endif
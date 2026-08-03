#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <stdlib.h>

#include "config.h"
#include "http_common.h"


typedef struct request_s {
    char method[MAX_METHOD_LEN];
    char path[MAX_PATH_LEN];
    char version[MAX_VERSION_LEN];

    header_t headers[MAX_HEADER_NB];
    size_t header_count;

    char body[MAX_BODY_LEN];
    size_t body_len;

    char connection_type[MAX_HEADER_VALUE_SIZE];

} request_t;



request_t *init_http_request(void);
void free_http_request(request_t *req);


#endif
#ifndef HTTP_CGI_OUTPUT
#define HTTP_CGI_OUTPUT


#include <stddef.h>

#include "config.h"
#include "http_common.h"
#include "http_codes.h"


typedef struct http_cgi_output_s {

    http_status_e status;

    header_t headers[MAX_HEADER_NB];
    size_t header_count;

    char body[MAX_BODY_LEN];
    size_t body_len;

    char connection_type[MAX_HEADER_VALUE_SIZE];

} http_cgi_output_t;


#endif
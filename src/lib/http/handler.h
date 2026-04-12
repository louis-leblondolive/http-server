#ifndef HANDLER
#define HANDLER

#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "structures.h"
#include "response.h"

typedef struct Mime_type {
    char *ext;
    char *mime;
} mime_type ;

char *get_mime_type(char *path);

http_status handle_error(response *serv_resp, http_status err_status);
http_status handle_get(request *client_req, response *serv_resp, bool head_only);
http_status handle_options(response *serv_resp);

#endif
#ifndef HANDLER
#define HANDLER

#include <string.h>
#include <stdbool.h>

#include "structures.h"
#include "response.h"

typedef struct Mime_type {
    char *ext;
    char *mime;
} mime_type ;

int handle_error(response *serv_resp, int err_code);
int handle_get(request *client_req, response *serv_resp);

#endif
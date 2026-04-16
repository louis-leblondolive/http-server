#ifndef HANDLER
#define HANDLER

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#include "structures.h"
#include "response.h"
#include "parser.h"
#include "ring_buffer.h"
#include "printer.h"

typedef struct Mime_type {
    char *ext;
    char *mime;
} mime_type ;


char *get_mime_type(char *path);
http_status handle_error(config_infos *cfg_infos, response *serv_resp, http_status err_status);
http_status handle_get(config_infos *cfg_infos, request *client_req, response *serv_resp, bool head_only);
http_status handle_options(config_infos *cfg_infos, response *serv_resp);
http_status handle_cgi(config_infos *cfg_infos, request *client_req, response *serv_resp);

#endif
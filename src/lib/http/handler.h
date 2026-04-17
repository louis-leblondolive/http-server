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
int handle_error(config_infos *cfg_infos, http_status err_status);
int handle_get(config_infos *cfg_infos, request *client_req, bool head_only);
int handle_options(config_infos *cfg_infos);
int handle_cgi(config_infos *cfg_infos, request *client_req);

#endif
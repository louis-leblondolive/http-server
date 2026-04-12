#ifndef ROUTER
#define ROUTER

#include <unistd.h>
#include <sys/stat.h>

#include "structures.h"
#include "handler.h"

http_status route_request(config_infos *cfg_infos, request *client_req, response *serv_resp, http_status error_flag);

#endif
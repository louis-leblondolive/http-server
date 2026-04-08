#ifndef ROUTER
#define ROUTER

#include "structures.h"
#include "handler.h"

int route_request(config_infos *cfg_infos, request *client_req, response *serv_resp, int error_flag);

#endif
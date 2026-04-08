#ifndef ROUTER
#define ROUTER

#include "structures.h"
#include "handler.h"

int route_request(request *client_req, response *serv_resp, int error_flag);

#endif
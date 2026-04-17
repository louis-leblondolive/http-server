#ifndef ROUTER
#define ROUTER

#include <unistd.h>
#include <sys/stat.h>

#include "structures.h"
#include "printer.h"
#include "handler.h"

/**
 * @brief Request routing main entry point 
 * Checks request syntax and content, and delegates operations to handlers (GET, POST, etc.)
 * 
 * @param cfg_infos     Session configuration informations (including quiet or verbose mode)
 * @param client_req    Pointer to a parsed client request. Cannot be NULL
 * @param serv_resp     Pointer to the server response to fill. Must be pre-allocated. 
 * @param error_flag :  used to pinpoint error that could have occured during parsing 
 * @return              HTTP_OK if routing is successful, corresponding HTTP status otherwise 
 */
int route_request(config_infos *cfg_infos, request *client_req, http_status error_flag);

#endif
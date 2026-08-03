#ifndef ROUTER
#define ROUTER


#include "http_session.h"
#include "http_response.h"
#include "printer.h"
#include "handler.h"

/**
 * @brief Request routing main entry point.
 * Checks request syntax and content, and delegates operations to handlers (GET, POST, etc.).
 * 
 * @param cfg_infos     Configuration infos (verbosity). Destination socket must be set.
 * @param session       Pointer to a client session. Cannot be NULL.
 * @param serv_resp     Pointer to the server response for handlers to fill. Cannot be NULL.
 * @return              HTTP_OK if routing is successful, corresponding HTTP status otherwise.
 */
int route_request(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp);

#endif
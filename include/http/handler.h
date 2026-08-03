#ifndef HANDLER
#define HANDLER

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#include "http_codes.h"
#include "http_request.h"
#include "http_response.h"
#include "http_session.h"
#include "parser.h"
#include "ring_buffer.h"
#include "printer.h"


/**
 * @brief Handles an HTTP error.
 * @param cfg_infos    Configuration informations (Destination socket must be set).
 * @param session      A pointer to the client http session. 
 * @param serv_resp    A pointer to the server http response to fill. 
 * @param err_status   The error to send back.
 * @return 0 on successful handle, -1 otherwise.
 */
int handle_error(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp, http_status_e err_status);

/**
 * @brief Handles an HTTP GET request. 
 * @note Size of the requested file is not bounded. 
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @param session      A pointer to the client http session. 
 * @param serv_resp    A pointer to the server http response to fill. 
 * @param head_only     Set to true to send head only (Usefull to handle HEAD requests).
 * @return 0 on successful handle and send, -1 otherwise.
 */
int handle_get(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp, bool head_only);

/**
 * @brief Handles an HTTP OPTIONS request
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @param session      A pointer to the client http session. 
 * @param serv_resp    A pointer to the server http response to fill. 
 * @return 0 on successful handle and send, -1 otherwise.
 */
int handle_options(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp);

/**
 * @brief Handles a CGI request, using fork() and execve to execute external scripts. 
 * @note GET and POST CGI requests are both handled by this function. 
 * 
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @param session      A pointer to the client http session. 
 * @param serv_resp    A pointer to the server http response to fill. 
 * 
 * @return 0 on successful handle and send, -1 otherwise.
 * 
 * @warning Script response size is bounded by MAX_BODY_LEN, trying to go above will trigger an 
 * error. 
 */
int handle_cgi(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp);

#endif
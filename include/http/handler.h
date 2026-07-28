#ifndef HANDLER
#define HANDLER

#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#include "compat.h"
#include "structures.h"
#include "response.h"
#include "parser.h"
#include "ring_buffer.h"
#include "printer.h"

typedef struct Mime_type {
    char *ext;
    char *mime;
} mime_type ;

// --- UTILS ---------------   
/**
 * @brief Returns the MIME type of the file pointed to by `path`.
 * @note This function does not check path validity. 
 * @return The correponding MIME type, or "application/octet-stream" if unknown
 */
char *get_mime_type(char *path);

// --- HANDLE FUNCTIONS ---------------    
/**
 * @brief Handles an HTTP error.
 * @param cfg_infos    Configuration informations (Destination socket must be set).
 * @param err_status   The error to send back.
 * @return 0 on successful handle and send, -1 otherwise.
 */
int handle_error(config_infos *cfg_infos, http_status err_status);

/**
 * @brief Handles an HTTP GET request. 
 * @note Size of the requested file is not bounded. 
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @param client_req    Pointer to the client request.
 * @param head_only     Set to true to send head only (Usefull to handle HEAD requests).
 * @return 0 on successful handle and send, -1 otherwise.
 */
int handle_get(config_infos *cfg_infos, request *client_req, bool head_only);

/**
 * @brief Handles an HTTP OPTIONS request
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @return 0 on successful handle and send, -1 otherwise.
 */
int handle_options(config_infos *cfg_infos);

/**
 * @brief Handles a CGI request, using fork() and execl with environment variables to execute 
 * external scripts. 
 * @note GET and POST CGI requests are both handled by this function. 
 * 
 * @param cfg_infos     Configuration informations (Destination socket must be set).
 * @param client_req    Pointer to the client request.
 * @return 0 on successful handle and send, -1 otherwise.
 * 
 * @warning Script response size is bounded by MAX_BODY_LEN, trying to go above will trigger an 
 * error. 
 */
int handle_cgi(config_infos *cfg_infos, request *client_req);

#endif
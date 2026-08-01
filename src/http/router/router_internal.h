#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif

#ifndef HTTP_ROUTER_INTERNAL
#define HTTP_ROUTER_INTERNAL


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"
#include "http_codes.h"
#include "http_request.h"

/**
 * @brief Secures the path by adding root directory www as a prefix
 * Replaces root "/" with the default path 
 */
void assign_real_path(config_infos_t *cfg_infos, request_t *client_req);

/**
 * @brief Checks if path is only made of allowed characters 
 * @param path The path contained in the client request 
 */
bool path_is_valid(char *path);

/**
 * @brief Checks if file pointed to by path exists.
 * @param path The local path to the file (including www prefix)
 */
bool file_exists(char *path);

/**
 * @brief Checks if file is a descendant of the www_root directory to 
 *  avoid path traversal 
 * @param path The local path to the file (including www_root prefix)
 */
bool file_access_allowed(config_infos_t *cfg_infos, char *path);


/** 
 * @brief Checks if request content is correct
 * @return HTTP_OK if request content is valid, corresponding http_status otherwise 
 * @note Local path (including www prefix) must have been assigned to client request before calling
 */
http_status_e check_request(config_infos_t *cfg_infos, request_t *client_req);


#endif
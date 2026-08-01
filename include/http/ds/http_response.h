#ifndef HTTP_RESPONSE 
#define HTTP_RESPONSE 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "http_codes.h"


#define MAX_RESPONSE_STATUS_LEN (MAX_VERSION_LEN + 1 + MAX_CODE_LEN + 1 + MAX_REASON_LEN + 2 + 1)
#define MAX_RESPONSE_HEADERS_LEN ((MAX_HEADER_KEY_SIZE + 2 + MAX_HEADER_VALUE_SIZE + 2 + 1) * MAX_HEADER_NB)


typedef enum http_response_type {
    RAW_HTTP_RESP,
    FILE_HTTP_RESP
} http_response_type_e;


typedef struct http_response_s {

    char status[MAX_RESPONSE_STATUS_LEN];
    size_t status_len;

    char headers[MAX_RESPONSE_HEADERS_LEN];
    size_t headers_len;

    http_response_type_e type;
    
    union {
        char file_path[MAX_PATH_LEN];
        char raw_content[MAX_BODY_LEN];
    } content;

    size_t content_lenght;  // Number of bytes to send (header might contain a different value)


} http_response_t;


// ---------- RESPONSE MANAGEMENT -----------------------------------------------------------
/**
 * @brief Initialize all http response fields
 */
void init_http_response(http_response_t *serv_resp);

/**
 * @brief Initiates an http response version, code and reason fields following a given status.
 * @param serv_resp     Pointer to the server response, should be initialized 
 * @param status        Response status to set.
 * @return HTTP_OK if status fields were correctly completed, corresponding error status otherwise.
 */
http_status_e init_response_status(http_response_t *serv_resp, http_status_e status);

/**
 * @brief Adds a header to an http response.
 * @param serv_resp_hd  Pointer to the server response header should be added to.
 * @param key           Header key.
 * @param value         Header value.
 * @return HTTP_OK if header was correctly added, corresponding error status otherwise.
 */
http_status_e add_header(http_response_t *serv_resp, char *key, char *value);

/**
 * @brief Adds date and server headers to a given http response.
 * @return HTTP_OK upon success, corresponding error status otherwise.
 */
http_status_e init_response_default_headers(http_response_t *serv_resp);


http_status_e init_response_content(http_response_t *serv_resp, http_response_type_e resp_type, char *content, 
    size_t content_len, size_t content_len_header);


#endif
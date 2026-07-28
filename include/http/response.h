#ifndef RESPONSE
#define RESPONSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structures.h"
#include "printer.h"

typedef struct HTTP_Reason_Code {
    int code;
    char *reason; 
} http_reason_code;

// ---------- HTTP reasons and error codes --------------------------------------------
/**
 * @brief Returns a pointer to the HTTP reason & code associated to a given status.
 * @param status    HTTP status which reason & code should be determined.
 * @return Pointer to the corresponding HTTP reason & code, {0, Unknown} if status is not known.
 */
const http_reason_code *get_http_reason(http_status status);

/**
 * @brief Returns the HTTP status associated to a given code.
 * @param code The code associated to the status to determine.
 * @return Associated status, or HTTP_INTERNAL_ERROR if code is not known.
 */
http_status get_status_from_code(int code);

// ---------- Response handling ------------------------------------------------
/**
 * @brief Adds a header to a response head.
 * @param serv_resp_hd  Pointer to the server response head header should be added to.
 * @param key           Header key.
 * @param value         Header value.
 * @return HTTP_OK if header was correctly added, corresponding error status otherwise.
 */
http_status add_header(response_head *serv_resp_hd, char *key, char *value);

/**
 * @brief Initiates a response version, code and reason fields following a given status.
 * @param serv_resp_hd   Pointer to the server response head status fields shoud be initialized.
 * @param status         Response status to set.
 * @return HTTP_OK if status fields were correctly completed, corresponding error status otherwise.
 */
http_status init_response_status(response_head *serv_resp_hd, http_status status);

/**
 * @brief Adds date and server headers to a given response head.
 * @return HTTP_OK upon success, corresponding error status otherwise.
 */
http_status init_response_default_headers(response_head *serv_resp_hd);

/**
 * @brief Reads the content_len field and add a Content-Length header with the correponding value
 */
http_status init_response_content_length(response_head *serv_resp_hd);


// -------------- Send logic ---------------------------------------------------------------------
/**
 * @brief A derivative of the send() function that ensures the messages is sent entirely.
 * @note Calls to this function will block until all of the message is sent.
 * 
 * @param cfg_infos    Configuration file, with destination socket set. 
 * @param buf          The message to be sent.
 * @param buf_len      Message length.
 * @return 0 upon sucess, -1 otherwise.
 * 
 * @warning Destination socket must be set in cfg_infos.
 */
int send_raw_content(config_infos *cfg_infos, char *buf, size_t buf_len);

/**
 * @brief Sends response status and headers to the client.
 * @note Calls to this function will block until all of the message is sent.
 * 
 * @param cfg_infos     Configuration file, with destination socket set. 
 * @param serv_resp_hd  Server response head, with headers and status fields complete. 
 * @return  0 upon sucess, -1 otherwise.
 * 
 * @warning Destination socket must be set in cfg_infos.
 */
int send_response_head(config_infos *cfg_infos, response_head *serv_resp_hd);

#endif
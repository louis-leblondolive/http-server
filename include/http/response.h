#ifndef RESPONSE
#define RESPONSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "http_codes.h"
#include "printer.h"

// ---------- Response handling ------------------------------------------------


/**
 * @brief Reads the content_len field and add a Content-Length header with the correponding value
 */
//http_status_e init_response_content_length(response_head_t *serv_resp_hd);


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
//int send_raw_content(config_infos_t *cfg_infos, char *buf, size_t buf_len);

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
//int send_response_head(config_infos_t *cfg_infos, response_head_t *serv_resp_hd);

#endif
#ifndef PARSER
#define PARSER 

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "structures.h"
#include "ring_buffer.h"
#include "config.h"
#include "printer.h"


typedef enum Parsing_Request_State {
    REQ_PARSING_METHOD, 
    REQ_PARSING_METHOD_SEPARATOR,
    REQ_PARSING_PATH, 
    REQ_PARSING_PATH_SEPARATOR,
    REQ_PARSING_VERSION, 
    REQ_PARSING_HEADER_KEY,
    REQ_PARSING_HEADER_KEY_SEPARATOR,
    REQ_PARSING_HEADER_VALUE, 
    REQ_PARSING_BODY,
    REQ_PARSING_NEW_LINE,
    REQ_EXPECTING_LF,
    REQ_EXPECTING_FINAL_LF, 
    REQ_END_PARSING
    
} parsing_request_state ; 


typedef enum Parsing_Response_State {
    RESP_EXPECTING_LF, 
    RESP_PARSING_NEW_LINE,
    RESP_PARSING_HEADER_KEY,
    RESP_PARSING_HEADER_KEY_SEPARATOR,
    RESP_PARSING_HEADER_VALUE, 
    RESP_EXPECTING_FINAL_LF, 
    RESP_PARSING_BODY, 
    RESP_END_PARSING
} parsing_response_state ; 


/**
 * @brief Parses an http request from raw text to a filled request structure (see structures.h).
 * * The parsing support partial request reception, meaning it can be run several times on chunked 
 * requests. Parsing completion state is stored in a pointer-referenced boolean, so user must ensure
 * his request was entirely parsed before proceeding to routing. 
 * 
 * @warning All pointer parameters must be pre-allocated. The caller is responsible for their life cycle.
 * 
 * @param[in]      raw_request_buf     Ring buffer containing raw request.
 * @param[in, out] parsed_request      Pointer to the request that is currently being parsed.
 * @param[in]      bytes_received      Number of processable bytes in the buffer.
 * @param[in, out] total_bytes_parsed  Total amount of bytes processed for this request.
 * @param[in, out] pos                 Current writing position in the active parsing request field
 * @param[out]     parsing_complete    A pointer to a boolean indicating parsing completion state 
 * @param[in, out] parse_state         Current finite state machine parsing state 
 * 
 * @return Returns HTTP_OK if the chunk was parsed without error, or the corresponding 
 * HTTP error code otherwise. 
 * @note **Crucial**: A return of HTTP_OK does NOT mean the request is ready. 
 * Always check `*parsing_complete` before routing.
 */
http_status parse_raw_request(config_infos *cfg_infos, r_buffer *raw_request_buf, request *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos,
                            bool *parsing_complete, parsing_request_state *parse_state);




http_status parse_raw_cgi_response(config_infos *cfg_infos, r_buffer *raw_response_buf, 
                            response_head *parsed_response_head, char *parsed_resp_body, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_response_state *parse_state, bool *has_body_length);

#endif 
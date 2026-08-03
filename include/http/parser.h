#ifndef PARSER
#define PARSER 

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "http_request.h"
#include "http_cgi_output.h"
#include "ring_buffer.h"
#include "config.h"
#include "printer.h"


typedef enum parsing_request_state {
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
    
} parsing_request_state_e ; 


typedef enum parsing_cgi_output_state {
    CGI_OUT_EXPECTING_LF, 
    CGI_OUT_PARSING_NEW_LINE,
    CGI_OUT_PARSING_HEADER_KEY,
    CGI_OUT_PARSING_HEADER_KEY_SEPARATOR,
    CGI_OUT_PARSING_HEADER_VALUE, 
    CGI_OUT_EXPECTING_FINAL_LF, 
    CGI_OUT_PARSING_BODY, 
    CGI_OUT_END_PARSING
} parsing_cgi_output_state_e ; 


/**
 * @brief Parses an http request from raw text to a filled request structure (see structures.h).
 * * The parsing support partial request reception, meaning it can be run several times on chunked 
 * requests. Parsing completion state is stored in a pointer-referenced boolean, so user must ensure
 * his request was entirely parsed before proceeding to routing. 
 * 
 * @warning All pointer parameters must be pre-allocated. The caller is responsible for their life cycle.
 * 
 * @param[in]      cfg_infos           Configuration infos (verbosity).
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
http_status_e parse_raw_request(config_infos_t *cfg_infos, ring_buffer_t *raw_request_buf, request_t *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos,
                            bool *parsing_complete, parsing_request_state_e *parse_state);



/**
 * @brief Parses response from a cgi script, from raw text to a filled response structure (see structures.h).
 * * The parsing support partial response read, meaning it can be run several times on chunked 
 * responses. It also supports non HTTP formated outputs (as long as headers keys and values are separated by ':'). 
 * Parsing completion state is stored in a pointer-referenced boolean, so user must ensure
 * his request was entirely parsed before proceeding to send response. 
 * 
 * @warning All pointer parameters must be pre-allocated. The caller is responsible for their life cycle.
 * 
 * @param[in]      cfg_infos              Configuration infos (verbosity).
 * @param[in]      raw_response_buf       Ring buffer containing raw response.
 * @param[in, out] parsed_response_head   Pointer to the request that is currently being parsed.
 * @param[in]      bytes_received         Number of processable bytes in the buffer.
 * @param[in, out] total_bytes_parsed     Total amount of bytes processed for this request.
 * @param[in, out] pos                    Current writing position in the active parsing response field.
 * @param[out]     parsing_complete       A pointer to a boolean indicating parsing completion state. 
 * @param[in, out] parse_state            Current finite state machine parsing state.
 * 
 * @return Returns HTTP_OK if the chunk was parsed without error, or the corresponding 
 * HTTP error code otherwise. 
 * @note **Crucial**: A return of HTTP_OK does NOT mean the request is ready. 
 * Always check `*parsing_complete` before responding.
 */
http_status_e parse_raw_cgi_output(config_infos_t *cfg_infos, ring_buffer_t *raw_response_buf, 
                            http_cgi_output_t *parsed_cgi_output,
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_cgi_output_state_e *parse_state, bool *has_body_length);

#endif 
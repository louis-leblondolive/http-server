#ifndef PARSER
#define PARSER 

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "structures.h"
#include "ring_buffer.h"


typedef enum Parsing_State {
    PARSING_METHOD, 
    PARSING_METHOD_SEPARATOR,
    PARSING_PATH, 
    PARSING_PATH_SEPARATOR,
    PARSING_VERSION, 
    PARSING_HEADER_KEY,
    PARSING_HEADER_KEY_SEPARATOR,
    PARSING_HEADER_VALUE, 
    PARSING_BODY,
    PARSING_NEW_LINE,
    EXPECTING_LF,
    EXPECTING_FINAL_LF, 
    END_PARSING
    
} parsing_state ; 

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
http_status parse_raw_request(r_buffer *raw_request_buf, request *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos,
                            bool *parsing_complete, parsing_state *parse_state);


#endif 
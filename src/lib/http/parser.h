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
    EXPECTING_LF,       // i.e. expecting \n
    EXPECTING_FINAL_LF, 
    END_PARSING
    
} parsing_state ; 


http_status parse_raw_request(r_buffer *raw_request_buf, request *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos,
                            bool *parsing_complete, parsing_state *parse_state);
/*
    parse_raw_request parses an http request from text to struct Request

    return value : 
    Function will return 0 if call is successful 
    Otherwise, it will return the http error code corresponding to the error that occured
*/

void print_request(request *r);

#endif 
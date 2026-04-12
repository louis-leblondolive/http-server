#ifndef PARSER
#define PARSER 

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "structures.h"


typedef enum Parsing_State {
    PARSING_METHOD, 
    PARSING_PATH, 
    PARSING_VERSION, 
    PARSING_HEADER_KEY,
    PARSING_HEADER_VALUE, 
    PARSING_BODY,
    
} parsing_state ; 

typedef enum Char_Expect {
    ANY,
    EXPECTING_CR,       // i.e. expecting \r
    EXPECTING_LF,       // i.e. expecting \n
    EXPECTING_FINAL_LF
} char_expect;


http_status parse_raw_request(char *raw_request, request *parsed_request, int bytes_received);
/*
    parse_raw_request parses an http request from text to struct Request

    return value : 
    Function will return 0 if call is successful 
    Otherwise, it will return the http error code corresponding to the error that occured
*/

void print_request(request *r);

#endif 
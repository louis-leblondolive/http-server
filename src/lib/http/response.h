#ifndef RESPONSE
#define RESPONSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "structures.h"

typedef struct HTTP_Reason_Code {
    int code;
    char *reason; 
} http_reason_code;


const http_reason_code *get_http_reason(http_status status);
void print_reponse(response *r);
void reset_response(response *serv_resp);
http_status add_header(response *serv_resp, char *key, char *value);
http_status init_response_status(response *serv_resp, http_status status);
http_status init_response_content_length(response *serv_resp);
char *build_text_response(config_infos* cfg_infos, response *serv_resp);

#endif
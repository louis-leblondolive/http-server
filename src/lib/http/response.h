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


const http_reason_code *get_http_reason(http_status status);
http_status get_status_from_code(int code);

void reset_response(response_head *serv_resp_hd);
http_status add_header(response_head *serv_resp_hd, char *key, char *value);
http_status init_response_status(response_head *serv_resp_hd, http_status status);
http_status init_response_default_headers(response_head *serv_resp_hd);
http_status init_response_content_length(response_head *serv_resp_hd);
char *build_text_response_head(config_infos* cfg_infos, response_head *serv_resp_hd, size_t *raw_response_len);

int send_raw_content(config_infos *cfg_infos, char *buf, size_t buf_len);
int send_response_head(config_infos *cfg_infos, response_head *serv_resp_hd);

#endif
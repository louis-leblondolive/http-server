#ifndef RESPONSE
#define RESPONSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structures.h"

typedef struct HTTP_Reason_Code {
    int code;
    char *reason; 
} http_reason_code;


char *http_reason(int code);
void print_reponse(response *r);
void reset_response(response *serv_resp);
int add_header(response *serv_resp, char *key, char *value);
int init_response_defaults(response *serv_resp, int code);
int init_response_content_length(response *serv_resp);
char *build_text_response(response *serv_resp);

#endif
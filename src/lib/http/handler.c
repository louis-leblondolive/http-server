#include "handler.h"





int handle_error(response *serv_resp, int error_flag){

    reset_response(serv_resp);

    if (init_response_defaults(serv_resp, error_flag) != 0) return -1;

    char *reason = http_reason(error_flag);

    if (add_header(serv_resp, "Content-Type", "text/html") != 0) return -1;

    char body[128];
    snprintf(body, sizeof(body), "<h1>%d - %s</h1>", error_flag, reason);
    strcpy(serv_resp->body, body);

    if (init_response_content_length(serv_resp) != 0) return -1;

    return 0;
}
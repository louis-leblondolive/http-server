#include "handler.h"



int handle_error(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp, http_status_e err_status){

    if(!cfg_infos || !session || !serv_resp) return -1;

    if(cfg_infos->verbose) print_debug("Handler - Handling error\n");

    // --- Reset response -----
    init_http_response(serv_resp);

    // --- Build response head ------------
    if (init_response_status(serv_resp, err_status) != HTTP_OK) return -1;
    if (init_response_default_headers(serv_resp) != HTTP_OK) return -1;
    
    const http_reason_code_t *reason_code = get_http_reason(err_status);

    if (add_header(serv_resp, "Content-Type", "text/html") != HTTP_OK) return -1;

    if (add_header(serv_resp, "Connection", "close") != HTTP_OK) return -1;
    set_http_session_connection_type(session, CLOSE);

    // --- Build response body ------------
    if(err_status == HTTP_NO_CONTENT || err_status == HTTP_NOT_MODIFIED
        || reason_code->code / 100 == 1){

        if(init_response_content(serv_resp, RAW_HTTP_RESP, NULL, 0, 0) != HTTP_OK) return -1;
    }
    else {
        char body[128];
        snprintf(body, sizeof(body), "<h1>%d - %s</h1>", reason_code->code, reason_code->reason);

        if(init_response_content(serv_resp, RAW_HTTP_RESP, body, strlen(body), strlen(body)) != HTTP_OK) return -1;
    }
    
    if(init_response_crlf(serv_resp) != HTTP_OK) return -1;

    return 0;
}
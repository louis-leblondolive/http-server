#include "handler.h"



int handle_options(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp){

    set_http_session_connection_type(session, CLOSE);
    
    http_status_e cache_res;

    // Build response head
    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    cache_res = init_response_default_headers(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Allow", ALLOWED_METHODS);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Connection", "close");
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    // Build response content 
    cache_res = init_response_content(serv_resp, RAW_HTTP_RESP, NULL, 0, 0);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    // CRLF
    cache_res = init_response_crlf(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    return 0;
}
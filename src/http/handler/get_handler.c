#include "handler.h"
#include "handler_utils/handler_utils.h"


int handle_get(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp, bool head_only){


    http_status_e cache_res;
    
    // ---------- Build response head -----------------------------------------------
    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    cache_res = init_response_default_headers(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Content-Type", get_mime_type(session->client_req->path));
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    struct stat st;
    if(stat(session->client_req->path, &st) == -1){
        return handle_error(cfg_infos, session, serv_resp, HTTP_NOT_FOUND);
    }

    // init content
    size_t content_len = (head_only) ? 0 : st.st_size;
    cache_res = init_response_content(serv_resp, FILE_HTTP_RESP, session->client_req->path, content_len, content_len);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);
    
    // handle last modified header
    char last_modified[MAX_HEADER_VALUE_SIZE];
    struct tm tm_info;
    if(gmtime_r(&st.st_mtime, &tm_info) == NULL) handle_error(cfg_infos, session, serv_resp, HTTP_INTERNAL_ERROR);

    strftime(last_modified, sizeof(last_modified), "%a, %d %b %Y %H:%M:%S GMT", &tm_info);

    cache_res = add_header(serv_resp, "Last-Modified", last_modified);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);
    
    // Connection type header
    if(session->connection_type == CLOSE) cache_res = add_header(serv_resp, "Connection", "close");
    else cache_res = add_header(serv_resp, "Connection", "keep-alive");

    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);
    
    // CRLF
    cache_res = init_response_crlf(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, session, serv_resp, cache_res);

    return HTTP_OK;
}
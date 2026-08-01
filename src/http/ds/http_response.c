#include "http_response.h"



// ---------- RESPONSE MANAGEMENT -----------------------------------------------------------
void init_http_response(http_response_t *serv_resp){
    serv_resp->status_len = 0;
    serv_resp->headers_len = 0;
    serv_resp->type = RAW_HTTP_RESP;
    serv_resp->content_lenght = 0;
}


http_status_e init_response_status(http_response_t *serv_resp, http_status_e status){

    
    const http_reason_code_t *reason_code = get_http_reason(status);
    
    if(!serv_resp || !reason_code) return HTTP_INTERNAL_ERROR;

    if(strlen(HTTP_VERSION) > MAX_VERSION_LEN
    || strlen(reason_code->reason) > MAX_REASON_LEN) return HTTP_INTERNAL_ERROR;
    
    int n_print = snprintf(serv_resp->status, sizeof(serv_resp->status), "%s %d %s\r\n", 
                            HTTP_VERSION, reason_code->code, reason_code->reason);

    if(n_print < 0 || (size_t)n_print >= sizeof(serv_resp->status)) return HTTP_INTERNAL_ERROR;

    serv_resp->status_len = (size_t)n_print;

    return HTTP_OK;
}


http_status_e add_header(http_response_t *serv_resp, char *key, char *value){
    
    if(!serv_resp || !key || !value) return HTTP_INTERNAL_ERROR;
    if(serv_resp->headers_len > MAX_RESPONSE_HEADERS_LEN) return HTTP_INTERNAL_ERROR;

    size_t hd_size = strlen(key) + 2 + strlen(value) + 2 + 1;
    size_t available_buf_space = MAX_RESPONSE_HEADERS_LEN - serv_resp->headers_len;

    if(strlen(key) > MAX_HEADER_KEY_SIZE
    || strlen(value) > MAX_HEADER_VALUE_SIZE
    || hd_size > available_buf_space)
        return HTTP_INTERNAL_ERROR;
    
    int n_print = snprintf(serv_resp->headers + serv_resp->headers_len, hd_size, 
                            "%s: %s\r\n", key, value);

    if(n_print < 0 || (size_t)n_print >= hd_size) return HTTP_INTERNAL_ERROR;
    
    serv_resp->headers_len += (size_t)n_print;

    return HTTP_OK;
}


http_status_e init_response_default_headers(http_response_t *serv_resp){

    // Date header
    char date[64];
    time_t now = time(NULL);
    struct tm gmt;

    if(gmtime_r(&now, &gmt) == NULL) return HTTP_INTERNAL_ERROR;
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &gmt);

    if(add_header(serv_resp, "Date",  date) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    // Server header
    char server_info[256];
    snprintf(server_info, sizeof(server_info), "%s/%s", SERVER_NAME, SERVER_VERSION);
    if(add_header(serv_resp, "Server", server_info) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    return HTTP_OK;
}


http_status_e init_response_content(http_response_t *serv_resp, http_response_type_e resp_type, char *content, 
    size_t content_len, size_t content_len_header){

    if(!serv_resp) return HTTP_INTERNAL_ERROR;
    
    serv_resp->type = resp_type;

    // File content
    if(resp_type == FILE_HTTP_RESP){
        if(!content) serv_resp->content_lenght = 0;
        else{
            size_t path_buf_size = sizeof(serv_resp->content.file_path);

            if(strlen(content) >= path_buf_size) return HTTP_INTERNAL_ERROR;
            
            // copy file path
            strncpy(serv_resp->content.file_path, content, path_buf_size);
            serv_resp->content.file_path[path_buf_size - 1] = '\0';

            serv_resp->content_lenght = content_len;
        }
    }

    // Raw content 
    else {
        if(!content || content_len == 0) serv_resp->content_lenght = 0;
        else {
            if(content_len > sizeof(serv_resp->content.raw_content)) return HTTP_INTERNAL_ERROR;
            // copy raw content
            memcpy(serv_resp->content.raw_content, content, content_len);
            serv_resp->content_lenght = content_len;
        }
    }

    // Add content length header
    char clen_str[MAX_HEADER_VALUE_SIZE];
    int n_copy_clen = snprintf(clen_str, sizeof(clen_str), "%zu", content_len_header);

    if(n_copy_clen < 0 || (size_t)n_copy_clen >= sizeof(clen_str)) return HTTP_INTERNAL_ERROR;
    
    if(add_header(serv_resp, "Content-Length", clen_str) != HTTP_OK) return HTTP_INTERNAL_ERROR;
    
    return HTTP_OK;
}
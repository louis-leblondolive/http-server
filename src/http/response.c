#include "response.h"



// ---------- Response handling ------------------------------------------------




/*

http_status_e init_response_content_length(response_head_t *serv_resp_hd){

    // Must be called after filling response body 

    char content_len_str[32];
    snprintf(content_len_str, sizeof(content_len_str), "%zu", serv_resp_hd->content_len);

    http_status_e add_h_res = add_header(serv_resp_hd, "Content-Length", content_len_str);

    if(add_h_res != HTTP_OK){
        return add_h_res;
    }

    return HTTP_OK;
}


// -------------- Send logic ---------------------------------------------------------------------
*
 * @brief Generates the raw text associated to a response head.
 * @param cfg_infos         Pointer to configuration infos (verbosity).
 * @param serv_resp_hd      Pointer to the server head response used for the generation.
 * @param raw_response_len  Pointer where to assign raw response length
 * @return The text response, terminated by a '\0' (not counted in raw_response_len)
 * 
 * @warning This function returns a pointer, user must free it after usage. 
 * @note This function assumes that response length has been tested and is the right size.
 */
/*
static char *build_text_response_head(config_infos_t *cfg_infos, response_head_t *serv_resp_hd, 
    size_t *raw_response_len){
    
    size_t status_len = strlen(serv_resp_hd->version) + strlen(serv_resp_hd->code) + strlen(serv_resp_hd->reason) + 4;
    size_t headers_len = 0;

    for (int i = 0; i < serv_resp_hd->header_count; i++){
        headers_len += strlen(serv_resp_hd->headers[i].key) + strlen(serv_resp_hd->headers[i].value) + 4;
    }

    size_t total_len = status_len + headers_len + 2 + 1; // counting \r\n and final \0 (which will be removed)


    char *text_response = (char*)malloc(sizeof(char) * total_len);
    size_t cursor = 0;
    
    if(!text_response) return NULL;

    char status_line[status_len + 1];
    snprintf(status_line, status_len + 1, "%s %s %s\r\n", serv_resp_hd->version, 
        serv_resp_hd->code, serv_resp_hd->reason);
    
    for (size_t i = 0; i < status_len; i++){
        text_response[cursor] = status_line[i];
        cursor++;
    }

    if(cfg_infos->verbose) print_debug("Response - done writing status\n");
    
    for (int h = 0; h < serv_resp_hd->header_count; h++){
        
        char header_line[MAX_HEADER_KEY_SIZE + MAX_HEADER_VALUE_SIZE + 4];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n", serv_resp_hd->headers[h].key,
            serv_resp_hd->headers[h].value);

        int line_len = strlen(header_line);
        for (int i = 0; i < line_len; i++){
            text_response[cursor] = header_line[i];
            cursor++;
        }
    }
    
    if(cfg_infos->verbose) print_debug("Response - done writing headers\n");

    text_response[cursor] = '\r';
    cursor ++;
    text_response[cursor] = '\n';
    cursor ++;

    text_response[cursor] = '\0';
    
    *raw_response_len = cursor;

    return text_response;
}


int send_raw_content(config_infos_t *cfg_infos, char *buf, size_t buf_len){

    if(cfg_infos->verbose) print_debug("Sending raw content \n");

    size_t sent = 0;
    while(sent < buf_len){
        ssize_t n = send(cfg_infos->client_fd, (void*)buf + sent, buf_len - sent, 0);
        if(n == -1) return -1;
        sent += n;
    }
    
    if(cfg_infos->verbose) print_debug("Done sending raw content\n");

    return 0;
}


int send_response_head(config_infos_t *cfg_infos, response_head_t *serv_resp_hd){

    if(cfg_infos->verbose){
        print_debug("Response - Sending response head \n");
        print_response(serv_resp_hd);
    }

    size_t raw_head_len = 0;
    char *raw_head = build_text_response_head(cfg_infos, serv_resp_hd, &raw_head_len);

    if(!raw_head) return -1;

    int send_hd_res = send_raw_content(cfg_infos, raw_head, raw_head_len);
    free(raw_head);

    if(cfg_infos->verbose) print_debug("Response - Done sending response head \n");
    
    return send_hd_res;
}

*/
#include "response.h"

static const http_reason_code http_200 = {200, "Ok"};
static const http_reason_code http_201 = {201, "Created"};
static const http_reason_code http_204 = {204, "No Content"};
static const http_reason_code http_400 = {400, "Bad Request"};
static const http_reason_code http_401 = {401, "Unauthorized"};
static const http_reason_code http_403 = {403, "Forbidden"};
static const http_reason_code http_404 = {404, "Not Found"};
static const http_reason_code http_413 = {413, "Request Entity Too Large"};
static const http_reason_code http_414 = {414, "Request URI Too Long"};
static const http_reason_code http_418 = {418, "I'm a teapot"};
static const http_reason_code http_431 = {431, "Request Header Fields Too Large"};
static const http_reason_code http_500 = {500, "Internal Server Error"};
static const http_reason_code http_501 = {501, "Not Implemented"};
static const http_reason_code http_505 = {505, "HTTP Version Not Supported"};
static const http_reason_code http_unknown = {0, "Unknown"};


const http_reason_code *get_http_reason(http_status status){

    switch (status)
    {
    case HTTP_OK:
        return &http_200;
    case HTTP_CREATED:
        return &http_201;
    case HTTP_NO_CONTENT:
        return &http_204;

    case HTTP_BAD_REQUEST:
        return &http_400;
    case HTTP_UNAUTHORIZED:
        return &http_401;
    case HTTP_FORBIDDEN:
        return &http_403;
    case HTTP_NOT_FOUND:
        return &http_404;
    case HTTP_REQUEST_ENTITY_TOO_LARGE:
        return &http_413;
    case HTTP_URI_TOO_LONG:
        return &http_414;
    case HTTP_TEAPOT:
        return &http_418;
    case HTTP_HEADER_TOO_LARGE:
        return &http_431;

    case HTTP_INTERNAL_ERROR:
        return &http_500;
    case HTTP_NOT_IMPLEMENTED:
        return &http_501;
    case HTTP_VERSION_NOT_SUPPORTED:
        return &http_505;

    default:
        return &http_unknown;
    }


}


void print_reponse(response *r){
    printf("%s %s %s\n", r->version, r->code, r->reason);
    for (int i = 0; i < r->header_count; i++)
    {
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
    printf("%s\n", r->body);
    printf("\n");
}


void reset_response(response *serv_resp){
   memset(serv_resp, 0, sizeof(response));
}


http_status add_header(response *serv_resp, char *key, char *value){
    
    if(serv_resp->header_count >= MAX_HEADER_NB 
    || strlen(key) > MAX_HEADER_KEY_SIZE
    || strlen(value) > MAX_HEADER_VALUE_SIZE){
        return HTTP_INTERNAL_ERROR;
    }

    header new_hd;
    strcpy(new_hd.key, key);
    strcpy(new_hd.value, value);

    serv_resp->headers[serv_resp->header_count] = new_hd;
    serv_resp->header_count ++;

    return HTTP_OK;
}


http_status init_response_status(response *serv_resp, http_status status){

    if(strlen(HTTP_VERSION) > MAX_VERSION_LEN) return HTTP_INTERNAL_ERROR;
    strcpy(serv_resp->version, HTTP_VERSION);

    const http_reason_code *reason_code = get_http_reason(status);

    const char *reason = reason_code->reason;
    if(strlen(reason) > MAX_REASON_LEN) return HTTP_INTERNAL_ERROR;
    strcpy(serv_resp->reason, reason);


    char code_str[16];
    snprintf(code_str, sizeof(code_str), "%d", reason_code->code);
    if(strlen(code_str) > MAX_CODE_LEN) return HTTP_INTERNAL_ERROR;
    strcpy(serv_resp->code, code_str);
    

    char date[64];
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    if(add_header(serv_resp, "Date",  date) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    return HTTP_OK;
}


http_status init_response_content_length(response *serv_resp){

    // Must be called after filling response body 

    char content_len_str[32];
    snprintf(content_len_str, sizeof(content_len_str), "%ld", strlen(serv_resp->body));

    http_status add_h_res = add_header(serv_resp, "Content-Length", content_len_str);

    if(add_h_res != HTTP_OK){
        return add_h_res;
    }

    return HTTP_OK;
}


char *build_text_response(config_infos* cfg_infos, response *serv_resp){
    // assume that request length has been tested and is the right size
    // returns a pointer : free must be used upon usage 
    
    size_t status_len = strlen(serv_resp->version) + strlen(serv_resp->code) + strlen(serv_resp->reason) + 4;
    size_t body_len = strlen(serv_resp->body);
    size_t headers_len = 0;

    for (int i = 0; i < serv_resp->header_count; i++){
        headers_len += strlen(serv_resp->headers[i].key) + strlen(serv_resp->headers[i].value) + 4;
    }

    size_t total_len = status_len + body_len + headers_len + 2 + 1; // counting \r\n and final \0 (which will be removed)


    char *text_response = (char*)malloc(sizeof(char) * total_len);
    int cursor = 0;
    
    if(!text_response) return NULL;

    char status_line[status_len + 1];
    snprintf(status_line, status_len + 1, "%s %s %s\r\n", serv_resp->version, serv_resp->code, serv_resp->reason);
    
    for (size_t i = 0; i < status_len; i++){
        text_response[cursor] = status_line[i];
        cursor++;
    }

    if(cfg_infos->verbose) printf("[DEBUG] response - done writing status\n");
    
    for (int h = 0; h < serv_resp->header_count; h++){
        
        char header_line[MAX_HEADER_KEY_SIZE + MAX_HEADER_VALUE_SIZE + 4];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n", serv_resp->headers[h].key, serv_resp->headers[h].value);

        int line_len = strlen(header_line);
        for (int i = 0; i < line_len; i++){
            text_response[cursor] = header_line[i];
            cursor++;
        }
    }
    
    if(cfg_infos->verbose) printf("[DEBUG] response - done writing headers\n");

    text_response[cursor] = '\r';
    cursor ++;
    text_response[cursor] = '\n';
    cursor ++;

    for (size_t i = 0; i < body_len; i++){
        text_response[cursor] = serv_resp->body[i];
        cursor++;
    }

    if(cfg_infos->verbose) printf("[DEBUG] response - done writing body\n");

    text_response[cursor] = '\0';

    return text_response;
}
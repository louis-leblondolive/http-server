#include "response.h"

static const http_reason_code http_r_codes[] = {
    {200, "Ok"},
    {201, "Created"},
    {204, "No Content"},

    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {413, "Request Entity Too Large"},
    {414, "Request URI Too Long"},
    {418, "I'm a teapot"},
    {431, "Request Header Fields Too Large"}, 

    {500, "Internal Server Error"}, 
    {501, "Not Implemented"},
    {505, "HTTP Version Not Supported"}
};

static const int http_r_code_count = 14;


char *http_reason(int code){
    for(int i = 0; i < http_r_code_count; i ++){
        if(http_r_codes[i].code == code){
            return http_r_codes[i].reason;
        }
    }
    return "Reason code not found";
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


int add_header(response *serv_resp, char *key, char *value){
    
    if(serv_resp->header_count >= MAX_HEADER_NB 
    || strlen(key) > MAX_HEADER_KEY_SIZE
    || strlen(value) > MAX_HEADER_VALUE_SIZE){
        return 500;
    }

    header new_hd;
    strcpy(new_hd.key, key);
    strcpy(new_hd.value, value);

    serv_resp->headers[serv_resp->header_count] = new_hd;
    serv_resp->header_count ++;

    return 0;
}


int init_response_defaults(response *serv_resp, int code){

    if(strlen(HTTP_VERSION) > MAX_VERSION_LEN) return 500;
    strcpy(serv_resp->version, HTTP_VERSION);

    
    char *reason = http_reason(code);
    if(strlen(reason) > MAX_REASON_LEN) return 500;
    strcpy(serv_resp->reason, reason);


    char code_str[16];
    snprintf(code_str, sizeof(code_str), "%d", code);
    if(strlen(code_str) > MAX_CODE_LEN) return 500;
    strcpy(serv_resp->code, code_str);

    return 0;
}


int init_response_content_length(response *serv_resp){

    char content_len_str[32];
    snprintf(content_len_str, sizeof(content_len_str), "%ld", strlen(serv_resp->body));

    int add_h_res = add_header(serv_resp, "Content-Length", content_len_str);

    if(add_h_res != 0){
        return add_h_res;
    }

    return 0;
}


char *build_text_response(response *serv_resp){
    // assume that request length has been tested and is the right size
    

    size_t status_len = strlen(serv_resp->version) + strlen(serv_resp->code) + strlen(serv_resp->reason) + 4;
    size_t body_len = strlen(serv_resp->body);
    size_t headers_len = 0;

    for (int i = 0; i < serv_resp->header_count; i++){
        headers_len += strlen(serv_resp->headers[i].key) + strlen(serv_resp->headers[i].value) + 4;
    }

    size_t total_len = status_len + body_len + headers_len + 2 + 1; // counting \r\n and final \0 (which will be removed)


    char *text_response = (char*)malloc(sizeof(char) * total_len);
    int cursor = 0;
    

    char status_line[status_len];
    snprintf(status_line, status_len + 1, "%s %s %s\r\n", serv_resp->version, serv_resp->code, serv_resp->reason);
    
    for (size_t i = 0; i < status_len; i++){
        text_response[cursor] = status_line[i];
        cursor++;
    }

    printf("response - done writing status\n");
    
    for (int h = 0; h < serv_resp->header_count; h++){
        
        char header_line[MAX_HEADER_KEY_SIZE + MAX_HEADER_VALUE_SIZE + 4];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n", serv_resp->headers[h].key, serv_resp->headers[h].value);

        int line_len = strlen(header_line);
        for (int i = 0; i < line_len; i++){
            text_response[cursor] = header_line[i];
            cursor++;
        }
    }
    
    printf("response - done writing headers\n");

    text_response[cursor] = '\r';
    cursor ++;
    text_response[cursor] = '\n';
    cursor ++;

    for (size_t i = 0; i < body_len; i++){
        text_response[cursor] = serv_resp->body[i];
        cursor++;
    }

    printf("response - done writing body\n");

    text_response[cursor] = '\0';

    return text_response;
}
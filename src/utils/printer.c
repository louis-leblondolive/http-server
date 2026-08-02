#include "printer.h"


void print_error(char *format, ...){
    va_list args;

    fprintf(stderr, BOLD_RED);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, RESET);
}

void print_info(char *format, ...){
    va_list args;

    printf(BOLD_BLUE);
    printf("[INFO] ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void print_debug(char *format, ...){
    va_list args;

    printf(BOLD_GREEN);
    printf("[DEBUG] ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void print_request(request_t *r){
    if(!r) return;

    printf("%s %s %s\n", r->method, r->path, r->version);
    for (int i = 0; i < r->header_count; i++){
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
    if(r->body_len <= 2000){
        printf("----- Body -----\n");
        if(strcmp(r->body, "\0") != 0) printf("%s\n", r->body);
        printf("----- End of Body -----\n");
    } else {
        printf("Long body (length > 2000)\n");
    }
    printf("\n");
}


void print_response(http_response_t *r){
    if(!r) return;

    printf("%.*s", (int)r->status_len, r->status);
    
    printf("%.*s", (int)r->headers_len, r->headers);

    printf("\n");
    printf("----- Body -----\n");
    if(r->type == RAW_HTTP_RESP) printf("%.*s\n", (int)r->content_lenght, r->content.raw_content);
    else printf("Pointing to : %s\n", r->content.file_path);
    printf("----- End of Body -----\n");
    printf("\n");
}
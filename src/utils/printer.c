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


void print_request(request *r){
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
}


void print_response(response_head *r){
    if(!r) return;

    printf("%s %s %s\n", r->version, r->code, r->reason);
    for (int i = 0; i < r->header_count; i++)
    {
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
}
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
    printf("%s %s %s\n", r->method, r->path, r->version);
    for (int i = 0; i < r->header_count; i++){
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
    printf("%s\n", r->body);
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
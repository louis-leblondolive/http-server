#ifndef PRINTER
#define PRINTER

#define BOLD_RED     "\033[1;31m"
#define BOLD_GREEN   "\033[1;32m"
#define BOLD_BLUE    "\033[1;34m"
#define RESET        "\033[0m"

#include <stdio.h>
#include <stdarg.h>

#include <structures.h>

void print_error(char *format, ...);
void print_info(char *format, ...);
void print_debug(char *format, ...);
void print_request(request *r);
void print_reponse(response *r);

#endif
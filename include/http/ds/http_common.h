#ifndef HTTP_COMMON
#define HTTP_COMMON


#include "config.h"

typedef struct header_s {
    char key[MAX_HEADER_KEY_SIZE];
    char value[MAX_HEADER_VALUE_SIZE];
} header_t ;

#endif
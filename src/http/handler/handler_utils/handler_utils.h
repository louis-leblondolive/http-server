#ifndef HTTP_HANDLER_UTILS
#define HTTP_HANDLER_UTILS

#include <string.h>

typedef struct mime_type_s {
    char *ext;
    char *mime;
} mime_type_t;


/**
 * @brief Returns the MIME type of the file pointed to by `path`.
 * @note This function does not check path validity. 
 * @return The correponding MIME type, or "application/octet-stream" if unknown
 */
char *get_mime_type(char *path);


#endif
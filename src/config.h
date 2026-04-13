#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

//  Server parameters
#define SERVER_NAME "l-olive"
#define SERVER_VERSION "1.0"
#define PORT "3490"
#define BACKLOG 10
#define DEFAULT_PATH "www/index.html"

//  HTTP parameters
#define MAX_REQUEST_LEN 4096    // keep requests under 4kb

#define MAX_HEADER_KEY_SIZE 64
#define MAX_HEADER_VALUE_SIZE 512

#define MAX_METHOD_LEN 12
#define MAX_PATH_LEN 512
#define MAX_VERSION_LEN 12
#define MAX_HEADER_NB 32
#define MAX_BODY_LEN 2048

#define MAX_CODE_LEN 4
#define MAX_REASON_LEN 64

#define ALLOWED_METHODS "GET, HEAD, OPTIONS"
#define HTTP_VERSION "HTTP/1.1"

typedef struct Config_infos {
    bool quiet;
    bool verbose;
} config_infos;

#endif
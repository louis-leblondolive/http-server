#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

//  Server parameters
#define SERVER_NAME "l-olive"
#define SERVER_VERSION "1.0"
#define PORT "3490"
#define BACKLOG 10
#define DEFAULT_PATH "index.html"   // www_root is included automatically 
#define TIMEOUT_SECONDS 5
#define TIMEOUT_MILLISECONDS 0

//  HTTP parameters
#define MAX_BODY_LEN          65536   // 64 KB
#define MAX_HEADER_KEY_SIZE   256
#define MAX_HEADER_VALUE_SIZE 4096
#define MAX_HEADER_NB         64
#define MAX_METHOD_LEN        16
#define MAX_PATH_LEN          2048
#define MAX_VERSION_LEN       16
#define MAX_CODE_LEN          4
#define MAX_REASON_LEN        128

#define MAX_REQUEST_LEN       (MAX_BODY_LEN + 16384)  // 64 KB body + 16 KB headers

#define ALLOWED_METHODS "GET, HEAD, OPTIONS, POST"
#define HTTP_VERSION "HTTP/1.1"

typedef struct Config_infos {

    // verbosity parameters
    bool quiet;
    bool verbose;

    // communication parameters
    int client_fd;
    char connection_type[MAX_HEADER_VALUE_SIZE];

    // session parameters
    char www_root[MAX_PATH_LEN];

} config_infos;

#endif
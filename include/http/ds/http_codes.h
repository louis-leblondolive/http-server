#ifndef HTTP_CODES
#define HTTP_CODES


typedef enum http_status {
    HTTP_OK,
    HTTP_CREATED, 
    HTTP_NO_CONTENT,

    HTTP_NOT_MODIFIED,

    HTTP_BAD_REQUEST,
    HTTP_UNAUTHORIZED,
    HTTP_FORBIDDEN,
    HTTP_METHOD_NOT_ALLOWED,
    HTTP_NOT_FOUND,
    HTTP_REQUEST_TIMEOUT,
    HTTP_REQUEST_ENTITY_TOO_LARGE,
    HTTP_URI_TOO_LONG, 
    HTTP_EXPECTATION_FAILED,
    HTTP_TEAPOT, 
    HTTP_HEADER_TOO_LARGE, 

    HTTP_INTERNAL_ERROR,
    HTTP_NOT_IMPLEMENTED,
    HTTP_BAD_GATEWAY,
    HTTP_VERSION_NOT_SUPPORTED

} http_status_e;


typedef struct http_reason_code_s {
    int code;
    char *reason; 
} http_reason_code_t;


/**
 * @brief Returns a pointer to the HTTP reason & code associated to a given status.
 * @param status    HTTP status which reason & code should be determined.
 * @return Pointer to the corresponding HTTP reason & code, {0, Unknown} if status is not known.
 */
const http_reason_code_t *get_http_reason(http_status_e status);

/**
 * @brief Returns the HTTP status associated to a given code.
 * @param code The code associated to the status to determine.
 * @return Associated status, or HTTP_INTERNAL_ERROR if code is not known.
 */
http_status_e get_status_from_code(int code);


#endif 
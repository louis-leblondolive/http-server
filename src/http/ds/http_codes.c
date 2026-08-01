#include "http_codes.h"



static const http_reason_code_t http_200 = {200, "Ok"};
static const http_reason_code_t http_201 = {201, "Created"};
static const http_reason_code_t http_204 = {204, "No Content"};
static const http_reason_code_t http_304 = {304, "Not Modified"};
static const http_reason_code_t http_400 = {400, "Bad Request"};
static const http_reason_code_t http_401 = {401, "Unauthorized"};
static const http_reason_code_t http_403 = {403, "Forbidden"};
static const http_reason_code_t http_404 = {404, "Not Found"};
static const http_reason_code_t http_405 = {405, "Method Not Allowed"};
static const http_reason_code_t http_408 = {408, "Request Time-out"};
static const http_reason_code_t http_413 = {413, "Request Entity Too Large"};
static const http_reason_code_t http_414 = {414, "Request URI Too Long"};
static const http_reason_code_t http_417 = {417, "Expectation Failed"};
static const http_reason_code_t http_418 = {418, "I'm a teapot"};
static const http_reason_code_t http_431 = {431, "Request Header Fields Too Large"};
static const http_reason_code_t http_500 = {500, "Internal Server Error"};
static const http_reason_code_t http_501 = {501, "Not Implemented"};
static const http_reason_code_t http_502 = {502, "Bad Gateway"};
static const http_reason_code_t http_505 = {505, "HTTP Version Not Supported"};
static const http_reason_code_t http_unknown = {0, "Unknown"};


const http_reason_code_t *get_http_reason(http_status_e status){

    switch (status)
    {
    case HTTP_OK:
        return &http_200;
    case HTTP_CREATED:
        return &http_201;
    case HTTP_NO_CONTENT:
        return &http_204;

    case HTTP_NOT_MODIFIED:
        return &http_304;

    case HTTP_BAD_REQUEST:
        return &http_400;
    case HTTP_UNAUTHORIZED:
        return &http_401;
    case HTTP_FORBIDDEN:
        return &http_403;
    case HTTP_NOT_FOUND:
        return &http_404;
    case HTTP_METHOD_NOT_ALLOWED:
        return &http_405;
    case HTTP_REQUEST_TIMEOUT:
        return &http_408;
    case HTTP_REQUEST_ENTITY_TOO_LARGE:
        return &http_413;
    case HTTP_URI_TOO_LONG:
        return &http_414;
    case HTTP_EXPECTATION_FAILED:
        return &http_417;
    case HTTP_TEAPOT:
        return &http_418;
    case HTTP_HEADER_TOO_LARGE:
        return &http_431;

    case HTTP_INTERNAL_ERROR:
        return &http_500;
    case HTTP_NOT_IMPLEMENTED:
        return &http_501;
    case HTTP_BAD_GATEWAY:
        return &http_502;
    case HTTP_VERSION_NOT_SUPPORTED:
        return &http_505;

    default:
        return &http_unknown;
    }
}


http_status_e get_status_from_code(int code){
    switch (code)
    {
    case 200: return HTTP_OK;
    case 201: return HTTP_CREATED;
    case 204: return HTTP_NO_CONTENT;
    case 304: return HTTP_NOT_MODIFIED;
    case 400: return HTTP_BAD_REQUEST;
    case 401: return HTTP_UNAUTHORIZED;
    case 403: return HTTP_FORBIDDEN;
    case 404: return HTTP_NOT_FOUND;
    case 405: return HTTP_METHOD_NOT_ALLOWED;
    case 408: return HTTP_REQUEST_TIMEOUT;
    case 413: return HTTP_REQUEST_ENTITY_TOO_LARGE;
    case 414: return HTTP_URI_TOO_LONG;
    case 417: return HTTP_EXPECTATION_FAILED;
    case 418: return HTTP_TEAPOT;
    case 431: return HTTP_HEADER_TOO_LARGE;
    case 500: return HTTP_INTERNAL_ERROR;
    case 501: return HTTP_NOT_IMPLEMENTED;
    case 502: return HTTP_BAD_GATEWAY;
    case 505: return HTTP_VERSION_NOT_SUPPORTED;
    default:  return HTTP_INTERNAL_ERROR;
    }
}
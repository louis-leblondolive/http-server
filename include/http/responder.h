#ifndef HTTP_RESPONDER
#define HTTP_RESPONDER


#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "net_send.h"
#include "http_response.h"

bool http_resp_qnode_is_sent(http_response_qnode_t *node);
int send_http_resp_qnode(int client_fd, http_response_qnode_t *node);
int send_http_resp_queue(int client_fd, http_response_queue_t *queue);

#endif
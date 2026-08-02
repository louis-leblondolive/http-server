#include "responder.h"



bool http_resp_qnode_is_sent(http_response_qnode_t *node){
    
    if(!node) return false;

    return node->response.status_len == node->n_status_sent
        && node->response.headers_len == node->n_headers_sent
        && node->response.content_lenght == node->n_content_sent;
}


int send_http_resp_qnode(int client_fd, http_response_qnode_t *node){

    if(client_fd < 0 || !node) return -1; 

    // Status send incomplete
    if(node->n_status_sent < node->response.status_len){

        ssize_t n_sent = send(client_fd, node->response.status + node->n_status_sent,
                                node->response.status_len - node->n_status_sent, 0);
        
        if(n_sent < 0) return -1;

        node->n_status_sent += (size_t)n_sent;
        if(node->n_status_sent < node->response.status_len) return 0;
    }

    // Headers send incomplete
    if(node->n_headers_sent < node->response.headers_len){

        ssize_t n_sent = send(client_fd, node->response.headers + node->n_headers_sent, 
                                node->response.headers_len - node->n_headers_sent, 0);

        if(n_sent < 0) return -1; 
        
        node->n_headers_sent += (size_t)n_sent;
        if(node->n_headers_sent < node->response.headers_len) return 0; 
    }

    // Content send incomplete
    if(node->n_content_sent >= node->response.content_lenght) return 0;

    ssize_t n_sent;
    switch(node->response.type){

        case RAW_HTTP_RESP:
            n_sent = send(client_fd, node->response.content.raw_content + node->n_content_sent,
                            node->response.content_lenght - node->n_content_sent, 0);
            
            if(n_sent < 0) return -1;

            node->n_content_sent += (size_t)n_sent;
            break;

        
        case FILE_HTTP_RESP:
            if(node->file_fd < 0){
                node->file_fd = open(node->response.content.file_path, O_RDONLY);
                if(node->file_fd < 0) return -1;
            }
            
            n_sent = net_sendfile(node->file_fd, client_fd, (off_t)node->n_content_sent, 
                            (off_t)(node->response.content_lenght - node->n_content_sent));

            if(n_sent < 0) return -1;
            
            node->n_content_sent += (size_t)n_sent;
            break;

        default:
            break; /* UNREACHABLE */
    }

    return 0;
}


int send_http_resp_queue(int client_fd, http_response_queue_t *queue){

    if(client_fd < 0 || !queue) return -1;
    while(!http_resp_queue_is_empty(queue)){

        int node_send_res = send_http_resp_qnode(client_fd, queue->head);

        if(node_send_res != 0) return node_send_res;
        if(!http_resp_qnode_is_sent(queue->head)) return 0;

        http_response_qnode_t *pop = http_resp_queue_pop(queue);
        http_resp_qnode_free(pop);
    }

    return 0;
}
#include "http_session.h"



// ----- SESSION LIFECYCLE ----------------------------------------
http_session_t *open_http_session(int client_fd){

    if(client_fd < 0) return NULL;
    
    print_info("Opening session (sock fd %d)\n", client_fd);

    http_session_t *session = (http_session_t*)malloc(sizeof(http_session_t));
    if(!session) return NULL;

    session->client_fd = client_fd;
    
    if(init_session_timer(session) != 0){
        free(session); return NULL;
    };

    session->connection_type = KEEP_ALIVE;

    // Request infos 
    session->request_raw_buffer = init_ring_buffer(2 * MAX_REQUEST_LEN);
    if(!session->request_raw_buffer){ 
        if(session->timer_fd >= 0) close(session->timer_fd);
        free(session); 
        return NULL; 
    }
    
    session->client_req = init_http_request();
    if(!session->client_req){
        if(session->timer_fd >= 0) close(session->timer_fd);
        free_ring_buffer(session->request_raw_buffer);
        free(session);
        return NULL;
    }

    session->parsing_complete = false;
    session->parse_res = HTTP_OK;
    session->parse_state = REQ_PARSING_METHOD;
    session->total_bytes_parsed = 0;
    session->pos = 0;

    // Response infos
    session->resp_queue = http_resp_queue_init();
    if(!session->resp_queue){
        if(session->timer_fd >= 0) close(session->timer_fd);
        free_ring_buffer(session->request_raw_buffer);
        free_http_request(session->client_req);
        free(session);
        return NULL;
    }

    return session;
}


void close_http_session(http_session_t *session){

    print_info("Closing session (sock fd %d)\n", session->client_fd);

    if(!session) return;

    // try to flush session response queue
    send_http_resp_queue(session->client_fd, session->resp_queue);
    http_resp_queue_free(session->resp_queue);

    close(session->client_fd);
    close_session_timer(session);

    free_ring_buffer(session->request_raw_buffer);
    free_http_request(session->client_req);

    free(session);
}


void set_http_session_connection_type(http_session_t *session, connection_type_e connection_type){

    if(session->connection_type != CLOSE){
        session->connection_type = connection_type;
    }
}


int reset_http_session_request_info(http_session_t *session){

    if(!session) return -1;

    session->client_req->method[0] = '\0';
    session->client_req->path[0] = '\0';
    session->client_req->version[0] = '\0';
    session->client_req->body[0] = '\0';
    session->client_req->connection_type[0] = '\0';

    for (size_t i = 0; i < MAX_HEADER_NB; i++){
        session->client_req->headers[i].key[0] = '\0';
        session->client_req->headers[i].value[0] = '\0';
    }

    session->client_req->header_count = 0;
    session->client_req->body_len = 0;

    session->parsing_complete = false;
    session->parse_res = HTTP_OK;
    session->parse_state = REQ_PARSING_METHOD;
    session->total_bytes_parsed = 0;
    session->pos = 0;

    return 0;
}

// ----- SESSION TIMEOUT MANAGEMENT ---------------------------------
// --- macOS behaviour ----------
#ifdef __APPLE__

int init_session_timer(http_session_t *session){
    if(!session) return 1;
    session->timer_fd = -1;
    return 0;
}


int start_session_timer(http_session_t *session){
    if(!session) return 1;

    if(evt_init(&session->timer_event, TIMER_EVT, EVT_TIMER, (void*)session) != 0) return 1;

    return evt_register(session->client_fd, &session->timer_event, false);
}


int close_session_timer(http_session_t *session){
    if(!session) return 1;
    return evt_register(session->client_fd, &session->timer_event, true);
}

#endif  // __APPLE__


// --- Linux behaviour ----------
#ifdef __linux__

int init_session_timer(http_session_t *session){
    if(!session) return 1;

    session->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    return 0;
}


int start_session_timer(http_session_t *session){
    if(!session) return 1;

    struct itimerspec timeout = {0};
    timeout.it_value.tv_sec = TIMEOUT_SECONDS + (TIMEOUT_MILLISECONDS / 1000);
    timeout.it_value.tv_nsec = (TIMEOUT_MILLISECONDS % 1000) * 1000000;

    if(timerfd_settime(session->timer_fd, 0, &timeout, NULL) == -1) return 1;

    if(evt_init(&session->timer_event, TIMER_EVT, EVT_TIMER, (void*)session) != 0) return 1;

    return evt_register(session->timer_fd, &session->timer_event);
}


int close_session_timer(http_session_t *session){
    if(session->timer_fd > 0) close(session->timer_fd); 
    return 0;
}


#endif // __linux__
#include "http_session.h"



// ----- SESSION LIFECYCLE ----------------------------------------
http_session_t *open_http_session(int client_fd){

    if(client_fd < 0) return NULL;

    http_session_t *session = (http_session_t*)malloc(sizeof(http_session_t));
    if(!session) return NULL;

    session->request_raw_buffer = init_ring_buffer(2 * MAX_REQUEST_LEN);
    if(!session->request_raw_buffer){ 
        free(session); 
        return NULL; 
    }
    
    session->cur_req = (request_t*)malloc(sizeof(request_t));
    if(!session->cur_req){
        free_ring_buffer(session->request_raw_buffer);
        free(session);
        return NULL;
    }

    if(init_session_timer(session) != 0){
        free_ring_buffer(session->request_raw_buffer);
        free_http_request(session->cur_req);
        free(session);
        return NULL;
    };

    session->client_fd = client_fd;
    session->events = 0;

    return session;
}


void close_http_session(http_session_t *session){

    if(!session) return;

    free_ring_buffer(session->request_raw_buffer);
    free_http_request(session->cur_req);

    close(session->client_fd);
    if(session->timer_fd >= 0) close(session->timer_fd);

    free(session);
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

    event_t evt;
    if(evt_init(&evt, TIMER_EVT, EVT_TIMER, (void*)session) != 0) return 1;

    return evt_register(session->client_fd, &evt);
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

    event_t evt; 
    if(evt_init(&evt, TIMER_EVT, EVT_TIMER, (void*)session) != 0) return 1;

    return evt_register(session->timer_fd, &evt);
}


#endif // __linux__
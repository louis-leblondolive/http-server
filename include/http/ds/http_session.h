#ifndef HTTP_SESSION
#define HTTP_SESSION 


#include <unistd.h>

#include "evt_queue.h"
#include "http_request.h"
#include "ring_buffer.h"


typedef struct http_session_s {

    int client_fd; 
    int timer_fd;

    uint32_t events;

    ring_buffer_t *request_raw_buffer; 
    request_t *cur_req;

} http_session_t;


// ----- SESSION LIFECYCLE ----------------------------------------
http_session_t *open_http_session(int client_fd);
void close_http_session(http_session_t *session);


// ----- SESSION TIMEOUT MANAGEMENT -------------------------------
#ifdef __linux__
#include <sys/timerfd.h>
#endif 

// only useful under linux 
int init_session_timer(http_session_t *session);

// restarts the timer if it is already running 
int start_session_timer(http_session_t *session);



#endif
#ifndef HTTP_SESSION
#define HTTP_SESSION 


#include <unistd.h>
#include <stdbool.h>

#include "evt_queue.h"
#include "http_request.h"
#include "http_response.h"
#include "ring_buffer.h"
#include "parser.h"

typedef enum connection_type {
    KEEP_ALIVE,
    CLOSE
} connection_type_e;


typedef struct http_session_s {

    int client_fd; 
    event_t socket_event;

    int timer_fd;
    event_t timer_event; 

    connection_type_e connection_type; 

    // --- Request infos
        // data storage
    ring_buffer_t *request_raw_buffer; 
    request_t *client_req;

        // parsing infos
    bool parsing_complete; 
    http_status_e parse_res; 
    parsing_request_state_e parse_state; 
    size_t total_bytes_parsed; 
    size_t pos;

    // --- Response infos
    http_response_queue_t *resp_queue;
    

} http_session_t;


// ----- SESSION LIFECYCLE ----------------------------------------
http_session_t *open_http_session(int client_fd);
void close_http_session(http_session_t *session);
void set_http_session_connection_type(http_session_t *session, connection_type_e connection_type);


// ----- SESSION TIMEOUT MANAGEMENT -------------------------------
#ifdef __linux__
#include <sys/timerfd.h>
#endif 

// only useful under linux 
int init_session_timer(http_session_t *session);

// restarts the timer if it is already running 
int start_session_timer(http_session_t *session);



#endif
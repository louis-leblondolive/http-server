#ifndef EVT_QUEUE
#define EVT_QUEUE


#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include "config.h"
#include "ring_buffer.h"

// warning : no event_free() because void* field may contain elaborate stuff

#define EVT_READ    0b00001
#define EVT_WRITE   0b00010
#define EVT_TIMER   0b00100
#define EVT_CLOSE   0b01000
#define EVT_ERROR   0b10000


typedef enum event_type {
    SOCKET_EVT, 
    TIMER_EVT 
} event_type_e;


typedef struct event_s {
    
    event_type_e type; 
    uint16_t expect;
    void *data;

} event_t;


int evt_init(event_t *ev, event_type_e ev_type, uint16_t ev_expect, void *ev_data);
int evt_register(int ev_fd, event_t *ev, bool ev_delete);

int init_evt_queue(void);
void close_evt_queue(void);
int evt_queue_wait(event_t **event_list, int n_events);



#endif
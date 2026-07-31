#ifdef __linux__

#define _GNU_SOURCE

#include "evt_queue.h"

#include <sys/epoll.h>
#include <errno.h>


static int evt_queue = -1;


// ----- EVENT MANAGEMENT ------------------------------------------------------
int evt_init(event_t *ev, event_type_e ev_type, uint16_t ev_expect, void *ev_data){
    if(!ev) return 1; 

    ev->type = ev_type; 
    ev->expect = ev_expect;
    ev->data = ev_data;

    return 0;
}


int evt_register(int ev_fd, event_t *ev){

    if(!ev) return 1;

    struct epoll_event epl_ev = {0};
    epl_ev.data.ptr = (void*) ev; 
    
    uint32_t epl_ev_mask = 0;
    switch(ev->type){
        case SOCKET_EVT: 
            if(ev->expect & EVT_READ) epl_ev_mask |= EPOLLIN;
            if(ev->expect & EVT_WRITE) epl_ev_mask |= EPOLLOUT; 
            break;

        case TIMER_EVT: 
            epl_ev_mask = EPOLLIN;
            break;

        default: 
            break; /* UNREACHABLE */
    }

    epl_ev_mask |= EPOLLONESHOT;    
    epl_ev.events = epl_ev_mask;

    if(epoll_ctl(evt_queue, EPOLL_CTL_ADD, ev_fd, &epl_ev) == -1){

        if(errno == EEXIST && epoll_ctl(evt_queue, EPOLL_CTL_MOD, ev_fd, &epl_ev) != -1){
            return 0;
        }
        return 1;
    }

    return 0;
}


// ----- EVENT QUEUE MANAGEMENT ------------------------------------------------
int init_evt_queue(void){

    evt_queue = epoll_create1(0);
    if(evt_queue < 0) return 1;

    return 0;
}


void close_evt_queue(void){
    if(evt_queue >= 0) close(evt_queue);
}


int evt_queue_wait(event_t **event_list, int n_events){

    if(!event_list || n_events <= 0 || n_events > MAX_EVENTS) return -1;

    struct epoll_event epoll_evs[n_events];
    int n_ready = epoll_wait(evt_queue, epoll_evs, n_events, -1);

    for (int i = 0; i < n_ready; i++){
        
        event_list[i] = (event_t*)epoll_evs[i].data.ptr;
        event_list[i]->expect = 0;   

        // Connection closed or error
        if(epoll_evs[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)){
            event_list[i]->expect |= EVT_CLOSE;
        }

        // Timer event 
        if(event_list[i]->type == TIMER_EVT){
            event_list[i]->expect |= EVT_TIMER;
        }
        // Socket 
        else {
            if(epoll_evs[i].events & EPOLLIN) event_list[i]->expect |= EVT_READ;
            if(epoll_evs[i].events & EPOLLOUT) event_list[i]->expect |= EVT_WRITE;
        }
    }
    
    return n_ready;
}


#endif // end ifdef __linux__
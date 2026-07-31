#ifdef __APPLE__ 

#include "evt_queue.h"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>


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

    struct kevent k_ev_changes[2];
    int n_changes = 0;

    switch(ev->type){

        case SOCKET_EVT:
            if(ev->expect & EVT_READ){
                EV_SET(&k_ev_changes[n_changes], ev_fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, (void*) ev);
                n_changes ++;
            }
    
            if(ev->expect & EVT_WRITE){
                EV_SET(&k_ev_changes[n_changes], ev_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, (void*) ev);
                n_changes ++;
            }

            if(n_changes > 0 && kevent(evt_queue, k_ev_changes, n_changes, NULL, 0, NULL) == -1){
                return 1;
            }
            break;
        

        case TIMER_EVT: {
            int us_time_out = (TIMEOUT_SECONDS + (TIMEOUT_MILLISECONDS / 1000)) * 1000000 
                            + (TIMEOUT_MILLISECONDS % 1000) * 1000;

            EV_SET(&k_ev_changes[0], ev_fd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_USECONDS, us_time_out, (void*) ev);
            if(kevent(evt_queue, &k_ev_changes[0], 1, NULL, 0, NULL) == -1){
                return 1;
            }
            break;
        }
            
        default: 
            break;  /* UNREACHABLE */        
    }

    return 0;
}




// ----- EVENT QUEUE MANAGEMENT ------------------------------------------------
int init_evt_queue(void){
    
    evt_queue = kqueue();
    if(evt_queue < 0) return 1;

    return 0;
}


void close_evt_queue(void){
    if(evt_queue >= 0) close(evt_queue);
}


int evt_queue_wait(event_t **event_list, int n_events){

    if(!event_list || n_events <= 0 || n_events > MAX_EVENTS) return -1;

    struct kevent k_ev_list[n_events];
    int n_k_ready = kevent(evt_queue, NULL, 0, k_ev_list, n_events, NULL);

    int n_ready = 0;

    for (int i = 0; i < n_k_ready; i++){
        if(k_ev_list[i].filter == 0) continue;

        event_list[n_ready] = (event_t*)k_ev_list[i].udata;
        event_list[n_ready]->expect = 0;

        // Error or closed connection 
        if(k_ev_list[i].flags & EV_ERROR || k_ev_list[i].flags & EV_EOF){
            event_list[i]->expect |= EVT_CLOSE;
        }

        // Timer event 
        if(k_ev_list[i].filter == EVFILT_TIMER){ 
            event_list[i]->expect |= EVT_TIMER;
        }
        // Socket event
        else {
            if(k_ev_list[i].filter == EVFILT_READ) event_list[n_ready]->expect |= EVT_READ;
            if(k_ev_list[i].filter == EVFILT_WRITE) event_list[n_ready]->expect |= EVT_WRITE;
            
            for (int j = i + 1; j < n_k_ready; j++){
                if(k_ev_list[j].ident == k_ev_list[i].ident){
                    if(k_ev_list[j].filter == EVFILT_READ) event_list[n_ready]->expect |= EVT_READ;
                    if(k_ev_list[j].filter == EVFILT_WRITE) event_list[n_ready]->expect |= EVT_WRITE;

                    k_ev_list[j].filter = 0;
                }
            }
        }
        n_ready ++;
    }
    
    return n_ready;
}


#endif // end ifdef __APPLE__
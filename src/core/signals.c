#include "signals.h"



void sigchld_handler(int s){

    (void)s;

    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}


int init_signal_handlers(void){
    
    struct sigaction sa; 
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigchld_handler;

    return sigaction(SIGCHLD, &sa, NULL);
}
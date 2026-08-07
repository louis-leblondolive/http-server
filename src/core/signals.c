#include "signals.h"



void sigchld_handler(int s){

    (void)s;

    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}


int init_signal_handlers(void){
    
    struct sigaction sa_chld; 
    sa_chld.sa_flags = SA_RESTART;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_handler = sigchld_handler;

    struct sigaction sa_ignore; 
    sa_ignore.sa_flags = SA_RESTART;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_handler = SIG_IGN;

    if(sigaction(SIGCHLD, &sa_chld, NULL) != 0) return 1;
    if(sigaction(SIGPIPE, &sa_ignore, NULL) != 0) return 1;

    return 0;
}
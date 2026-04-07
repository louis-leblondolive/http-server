#ifndef LISTENER
#define LISTENER


#include <stdio.h>      
#include <stdlib.h>      
#include <errno.h>       
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/wait.h>    
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <signal.h>      
#include <unistd.h>      

#include "config.h"
#include "utils.h"

/*
    Listens for any incoming connections and manages accepts
    The function contains an infinite loop : it will never return 
*/
void listener(int sock_fd);

#endif
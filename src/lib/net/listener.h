#ifndef LISTENER
#define LISTENER


#include <stdio.h>      
#include <stdlib.h>      
#include <stdbool.h>
#include <string.h>
#include <errno.h>       
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/wait.h>    
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <signal.h>      
#include <unistd.h>      

#include "config.h"
#include "structures.h"

#include "utils.h"
#include "ring_buffer.h"
#include "parser.h"
#include "router.h"
#include "response.h"

/*
    Listens for any incoming connections and manages accepts
    The function contains an infinite loop : it will never return 
*/
void listener(config_infos *cfg_infos, int sock_fd);

#endif
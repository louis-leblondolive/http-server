#ifndef SETUP
#define SETUP

#include <stdio.h>       
#include <string.h>      
#include <sys/types.h>   
#include <sys/socket.h>  
#include <netdb.h>       

#include "config.h"
#include "printer.h"

/**
 * @brief Creates and configures server socket.
 * @return  Returns a socket file descriptor, or -1 on failure 
 */
    
int setup_server(void);

#endif
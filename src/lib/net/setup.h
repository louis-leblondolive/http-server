#ifndef SETUP
#define SETUP

#include <stdio.h>       
#include <string.h>      
#include <sys/types.h>   
#include <sys/socket.h>  
#include <netdb.h>       

#include "config.h"

/*
    Creates and configures server socket.
    Returns a socket file descriptor, or -1 if failure 
*/
int setup_server(void);

#endif
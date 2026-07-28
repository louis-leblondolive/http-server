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
#include "printer.h"
#include "parser.h"
#include "router.h"
#include "response.h"

/**
 * @brief Main loop listening for any incoming connections and managing client processes.
 * * Each new connection is handled in a child process using fork() to isolate execution, 
 * especially for external scripts (CGI).
 * * HTTP/1.1 keep-alive persistence using an isolated ring buffer for memory efficient management. 
 *
 * @param config_infos  Session configuration information (verbosity)
 * @param sock_fd       Server socked file descriptor. Must be initialized 
 * and bound (see setup.c)
 *
 * * @note This function contains an infinite loop and is not expected to return 
 * unless a fatal signal is received.
 */
void listener(config_infos *cfg_infos, int sock_fd);

#endif
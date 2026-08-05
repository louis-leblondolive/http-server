#ifndef REACTOR
#define REACTOR


#include <stdio.h>      
#include <stdlib.h>      
#include <stdbool.h>
#include <string.h>
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/socket.h>  
 
#include "config.h"
#include "evt_queue.h"
#include "signals.h"

#include "http_session.h"
#include "http_response.h"

#include "ring_buffer.h"

#include "parser.h"
#include "router.h"
#include "responder.h"

#include "utils.h"
#include "printer.h"

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
void reactor(config_infos_t *cfg_infos, int server_fd);

#endif
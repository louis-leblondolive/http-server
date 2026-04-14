#include "listener.h"

/**
 * @brief Handler for signals, reaping zombie child processes.
 * Using waitpid and WNOHANG in a loop to gather every finished process without blocking 
 * the server. 
 */
static void sigchld_handler(int s){
    (void)s;  // silences unused variable warnings
    int saved_errno = errno;    // errno could be overridden 
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;    
}

/**
 * @brief A derivative of the send() function that ensures the messages is sent entirely.
 * @warning Calls to this function will block until all of the message is sent.
 * 
 * @param fd    Destination socket file descriptor. 
 * @param buf   The message to be sent.
 * @param len   Message length.
 * @return      0 upon sucess, -1 otherwise.
 */
static int send_all(int fd, char *buf, size_t len){

    size_t sent = 0;
    while(sent < len){
        ssize_t n = send(fd, (void*)buf + sent, len - sent, 0);
        if(n == -1) return -1;
        sent += n;
    }
    return 0;
}


void listener(config_infos *cfg_infos, int sock_fd){

    struct sigaction sa; 
    int client_fd;          
    struct sockaddr_storage client_addr; 
    socklen_t client_sin_size;

    
    if(listen(sock_fd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    // Setting up signal handler 
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);   // no signal will be blocked during handler execution 
    sa.sa_flags = SA_RESTART;   // restarting interupted systems calls to avoid recv or accept EINTR error 
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    if (!cfg_infos->quiet) print_info("Server online, listening on port %s\n", PORT);

    // ---------  Server main loop ----------------------------------------------------------
    while(1){  

        client_sin_size = sizeof(client_addr);
        client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_sin_size);
        if(client_fd == -1){
            perror("accept");
            continue;
        }
        
        if (!cfg_infos->quiet) print_info("Server : got connection from %s\n", 
            sockaddr_in_addr_to_str(&client_addr));

        pid_t pid = fork();

        if(pid == -1){
            perror("server : fork");
            close(client_fd);
            continue;
        }

        if(pid == 0){    // Child process
            int exit_status = 0;

            close(sock_fd);
            
            struct timeval timeout = {TIMEOUT_SECONDS, TIMEOUT_MILLISECONDS};
            if(setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
                perror("server : setting client socket option");
                close(client_fd);
                exit(1);
            }

            r_buffer *raw_request_buf = init_ring_buffer(2 * MAX_REQUEST_LEN + 1);

            while(1){   // Client main loop 

                //  ----- Receive and parse data --------------------------------------------
                bool parsing_complete = false;
                bool peer_closed = false;

                request client_req;
                memset(&client_req, 0, sizeof(request));
                client_req.header_count = 0;
                client_req.body_len = 0;

                http_status parse_res = HTTP_OK;
                parsing_state parse_state = PARSING_METHOD;
                size_t total_bytes_parsed = 0;
                size_t pos = 0;
                ssize_t bytes_received = 0;

                while(!parsing_complete){   // Parsing loop 
                    // trying to parse remaining data
                    parse_res = parse_raw_request(raw_request_buf, &client_req, 
                        bytes_received, &total_bytes_parsed, &pos,
                        &parsing_complete, &parse_state);

                    if(parse_res != HTTP_OK || parsing_complete) break;
                    
                    // waiting for new data if parsing didn't work 
                    char raw_request[MAX_REQUEST_LEN];
                    memset(raw_request, 0, sizeof(raw_request));
                    bytes_received = recv(client_fd, raw_request, MAX_REQUEST_LEN, 0);

                    if(bytes_received == 0){    // Connection closed 
                        if (!cfg_infos->quiet){
                            print_info("Server : peer closed its half side of the connection");
                        } 
                        exit_status = 0;
                        peer_closed = true;
                        break;
                    }
                    if(bytes_received == -1){   // Error during reception 
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            parse_res = HTTP_REQUEST_TIMEOUT;
                            break;
                        }
                        else {
                            perror("server :  recv");
                        } 
                        exit_status = 1;
                        break;
                    }

                    // Copy raw request to buffer 
                    if (write_string_in_r_buffer(raw_request_buf, raw_request, bytes_received) != 0){
                        exit_status = 1;
                        break;
                    }
                }

                // Client communication interuption during data reception 
                if(peer_closed || (!parsing_complete && parse_res == HTTP_OK) 
                || (!parsing_complete && parse_res == HTTP_REQUEST_TIMEOUT)){
                    break;
                }   

            
                // Displaying request
                if(!cfg_infos->quiet){
                    print_info("Parsed request (peer_closed=%d, parsing_complete=%d, parse_res=%d) : \n",
                        peer_closed, parsing_complete, parse_res);
                    print_request(&client_req);
                }
           

                //  ----- Process request --------------------------------------------
                response serv_resp;
                memset(&serv_resp, 0, sizeof(response));
                if (route_request(cfg_infos, &client_req, &serv_resp, parse_res) != HTTP_OK){
                    print_error("Server error while routing request\n");
                    exit_status = 1;
                    break;
                };
            
                // Displaying response
                if(!cfg_infos->quiet){
                    print_info("Processed request into response :\n");
                    print_reponse(&serv_resp);
                }

                //  ----- Respond --------------------------------------------------
                size_t raw_response_len = 0;
                char *raw_response = build_text_response(cfg_infos, &serv_resp, &raw_response_len);
                if(!raw_response){
                    print_error("Server couldn't build text response\n");
                    exit_status = 1;
                    break;
                }

                // Displaying raw response 
                if(cfg_infos->verbose){
                    print_debug("Sending raw response\n");
                    printf("%s\n", raw_response);
                }

                
                int send_res = send_all(client_fd, raw_response, raw_response_len);
                free(raw_response);

                if(send_res == -1){
                    perror("server : send");
                    exit_status = 1;
                    break;
                }

                // Exits if connection is close
                if(strcasecmp(serv_resp.connection_type, "close") == 0){
                    break;
                }

            }   // End of client keep-alive loop 
            
            if(!cfg_infos->quiet) print_info("Closing connection\n");
            free_ring_buffer(raw_request_buf);
            close(client_fd);
            exit(exit_status);

        }   // End of child process 

        // Server parent process
        close(client_fd);


    }   // Server main loop end 
}
#include "reactor.h"



void reactor(config_infos_t *cfg_infos, int server_fd){

    
    // ---------  Server initialization ----------------------------------------------------------
    if(init_evt_queue() != 0){
        print_error("Server error: couldn't initialize server event queue\n");
        exit(1);
    }

    http_session_t *server_session = open_http_session(server_fd);
    if(!server_session){
        print_error("Server: couldn't open server session");
        exit(1);
    }

    event_t server_evt;
    if(evt_init(&server_evt, SOCKET_EVT, EVT_READ, (void*)server_session) != 0
    || evt_register(server_fd, &server_evt) != 0){
        print_error("Server error: couldn't initialize server event system\n");
        close_evt_queue();
        close_http_session(server_session);
        exit(1);
    }

    if (!cfg_infos->quiet) print_info("Server online, listening on port %s\n", PORT);

    event_t *events[MAX_EVENTS];
    for (size_t i = 0; i < MAX_EVENTS; i++) events[i] = NULL;
    

    // ---------  Server main event loop ----------------------------------------------------------
    while(1){

        int n_event_ready = evt_queue_wait(events, MAX_EVENTS);

        for (int i = 0; i < n_event_ready; i++){
            
            event_t *evt = events[i];
            http_session_t *session = (http_session_t*) evt->data;

            // Server event 
            if(session->client_fd == server_fd){    

                if(evt->expect & EVT_CLOSE){        // Event error 
                    
                    print_error("Server: Fatal event system error\n");
                    break;
                }

                if(evt->expect & EVT_READ){         // New client available 

                    struct sockaddr_storage client_addr; 
                    socklen_t client_sin_size = sizeof(client_addr);

                    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_sin_size);
                    if(client_fd == -1){
                        perror("server: accept");
                        continue;
                    }
                    
                    http_session_t *client_session = open_http_session(client_fd);
                    if(!client_session){
                        print_error("Server error: couldn't open client session\n");
                        close(client_fd);
                        continue;
                    }
                    

                    char *str_client_addr = sockaddr_in_addr_to_str(&client_addr);
                    if (!cfg_infos->quiet) print_info("Server : got connection from %s\n", str_client_addr);
                    free(str_client_addr); 

                    if(evt_init(&client_session->socket_event, SOCKET_EVT, EVT_READ, (void*)client_session) != 0
                    || evt_register(client_fd, &client_session->socket_event) != 0){

                        print_error("Server error: Couldn't add client event to the event queue\n");
                        close_http_session(client_session);
                        continue;
                    }
                }

                if(evt_register(server_fd, &server_evt) != 0){
                    print_error("Server: Couldn't register to event queue\n");
                    break;
                }
            }


            // Client event 
            else {

                // Event error or connection closed by client 
                if(evt->expect & EVT_CLOSE){
                    close_http_session(session);
                    continue;
                }

                // Timeout 
                if(evt->type == TIMER_EVT && evt->expect & EVT_TIMER){
                    print_debug("Request timeout\n");
                    close_http_session(session);
                    continue;
                }

                // Available data
                if(evt->type == SOCKET_EVT && evt->expect & EVT_READ){

                    // --- Data reception ----------
                    char recv_buf[MAX_REQUEST_LEN];
                    ssize_t bytes_received = recv(session->client_fd, recv_buf, MAX_REQUEST_LEN, 0);
                    
                    if(bytes_received == 0){
                        if (!cfg_infos->quiet) print_info("Server : peer closed its half side of the connection\n");
                        close_http_session(session); continue;
                    }
                    if(bytes_received == -1){
                        perror("server: recv");
                        close_http_session(session); continue;
                    }

                    if(write_string_in_r_buffer(session->request_raw_buffer, recv_buf, bytes_received) != 0){
                        close_http_session(session); continue;
                    }


                    // --- Request parsing ----------
                    while(!r_buffer_is_empty(session->request_raw_buffer)){

                        session->parse_res = parse_raw_request(cfg_infos, session->request_raw_buffer,
                            session->client_req, bytes_received, &session->total_bytes_parsed, &session->pos, 
                            &session->parsing_complete, &session->parse_state);

                        if(!cfg_infos->quiet){
                            print_info("Parsed request (parsing_complete = %d, parse_res = %d):\n", 
                                session->parsing_complete, session->parse_res);
                            print_request(session->client_req);
                        }

                        // if session->parsing_complete, route and handle parsed request, then try to send the generated response and 
                        //      register to EVT_WRITE on failure
                        print_info("Implement routing and handling here \n");
                    }

                    print_info("Done parsing \n");

                    // if close connection continue
                    evt->expect = EVT_READ; 
                    if(evt_register(session->client_fd, evt) != 0){
                        print_error("Server error: couldn't add client event to the event queue\n");
                        close_http_session(session); continue;
                    }

                    if(start_session_timer(session) != 0){
                        print_error("Server error: couldn't start client timeout timer\n");
                        close_http_session(session);
                    }
                }

                // Sendable data 
                if(evt->type == SOCKET_EVT && evt->expect & EVT_WRITE){
                    print_debug("Write queueing not yet implemented\n");
                }

            } // end if server else client event  
        }
        

    }


    close_http_session(server_session);
    close_evt_queue();


    /*
    while(1){  

        client_sin_size = sizeof(client_addr);
        client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_sin_size);
        if(client_fd == -1){
            perror("accept");
            continue;
        }
        
        char *str_client_addr = sockaddr_in_addr_to_str(&client_addr);
        if (!cfg_infos->quiet) print_info("Server : got connection from %s\n", str_client_addr);
        free(str_client_addr); 
            

        pid_t pid = fork();

        if(pid == -1){
            perror("server : fork");
            close(client_fd);
            continue;
        }

        if(pid == 0){    // Child process
            int exit_status = 0;

            close(sock_fd);
            cfg_infos->client_fd = client_fd;
            
            struct timeval timeout = {TIMEOUT_SECONDS, TIMEOUT_MILLISECONDS};
            if(setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
                perror("server : setting client socket option");
                close(client_fd);
                exit(1);
            }

            ring_buffer_t *raw_request_buf = init_ring_buffer(2 * MAX_REQUEST_LEN);

            while(1){   // Client main loop 

                //  ----- Receive and parse data --------------------------------------------
                bool parsing_complete = false;
                bool peer_closed = false;

                request_t client_req;
                memset(&client_req, 0, sizeof(client_req));
                client_req.header_count = 0;
                client_req.body_len = 0;

                http_status_e parse_res = HTTP_OK;
                parsing_request_state_e parse_state = REQ_PARSING_METHOD;
                size_t total_bytes_parsed = 0;
                size_t pos = 0;
                ssize_t bytes_received = 0;

                char raw_request[MAX_REQUEST_LEN];
                memset(raw_request, 0, sizeof(raw_request));

                while(!parsing_complete){   // Parsing loop 

                    // trying to parse remaining data
                    parse_res = parse_raw_request(cfg_infos, raw_request_buf, &client_req, 
                        bytes_received, &total_bytes_parsed, &pos,
                        &parsing_complete, &parse_state);

                    if(parse_res != HTTP_OK || parsing_complete) break;

                    // loading new data 
                    bytes_received = recv(client_fd, raw_request, MAX_REQUEST_LEN, 0);

                    if(bytes_received == 0){    // Connection closed 
                        if (!cfg_infos->quiet){
                            print_info("Server : peer closed its half side of the connection\n");
                        } 
                        if (!parsing_complete && total_bytes_parsed == 0) {
                            parse_res = HTTP_BAD_REQUEST;  
                        }
                        exit_status = 0;
                        peer_closed = true;
                        break;
                    }
                    if(bytes_received == -1){   // Error during reception 
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            parse_res = HTTP_REQUEST_TIMEOUT;
                            exit_status = 0;
                        }
                        else {
                            perror("server :  recv");
                            exit_status = 1;
                        } 
                        break;
                    }

                    // Copy raw request to buffer 
                    if (write_string_in_r_buffer(raw_request_buf, raw_request, bytes_received) != 0){
                        exit_status = 1;
                        break;
                    }
                }

                if(peer_closed){
                    if(!cfg_infos->quiet) print_info("Client communication interuption during data reception\n");
                    break;
                }   

                if(exit_status == 1){
                    print_debug("Error during data reception\n");
                    break;
                }
            
                // Displaying request
                if(!cfg_infos->quiet){
                    print_info("Parsed request (peer_closed=%d, parsing_complete=%d, parse_res=%d) : \n",
                        peer_closed, parsing_complete, parse_res);
                    print_request(&client_req);
                }
                
                // Reset ring buffer
                memset(raw_request_buf->buf, 0, raw_request_buf->buf_size);
                raw_request_buf->read_pos = 0;
                raw_request_buf->write_pos = 0;

                //  ----- Process request --------------------------------------------
                
                if (route_request(cfg_infos, &client_req, parse_res) != 0){
                    print_error("Server error while routing request\n");
                    exit_status = 1;
                    break;
                };

                // Exits if connection is close
                if(strcasecmp(cfg_infos->connection_type, "close") == 0){
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


    }   // Server main loop end */
}
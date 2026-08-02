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
    || evt_register(server_fd, &server_evt, false) != 0){
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

                if(evt->expect & EVT_ERROR || evt->expect & EVT_CLOSE){        // Event error     
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
                    || evt_register(client_fd, &client_session->socket_event, false) != 0){

                        print_error("Server error: Couldn't add client event to the event queue\n");
                        close_http_session(client_session);
                        continue;
                    }

                    // Restart timeout timer
                    if(start_session_timer(client_session) != 0){
                        if(!cfg_infos->quiet) print_error("Server error: couldn't start client timeout timer\n");
                        close_http_session(client_session);
                    }
                }

                if(evt_register(server_fd, &server_evt, false) != 0){
                    print_error("Server: Couldn't register to event queue\n");
                    break;
                }
            }


            // Client event 
            else {

                // Event system failure 
                if(evt->expect & EVT_ERROR){
                    if(cfg_infos->verbose) print_error("Error during event filtering\n");
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
                        set_http_session_connection_type(session, CLOSE);
                    }
                    if(bytes_received == -1){
                        perror("server: recv");
                        close_http_session(session); continue;
                    }

                    if(write_string_in_r_buffer(session->request_raw_buffer, recv_buf, bytes_received) != 0){
                        if(cfg_infos->verbose) print_error("Couldn't write client data to ring buffer\n");
                        close_http_session(session); continue;
                    }


                    // --- Data processing ----------
                    bool close_session = false;

                    while(!r_buffer_is_empty(session->request_raw_buffer)){

                        // --- Parsing -----
                        session->parse_res = parse_raw_request(cfg_infos, session->request_raw_buffer,
                            session->client_req, bytes_received, &session->total_bytes_parsed, &session->pos, 
                            &session->parsing_complete, &session->parse_state);

                        if(cfg_infos->verbose){
                            print_info("Parsed request (parsing_complete = %d, parse_res = %d):\n", 
                                session->parsing_complete, session->parse_res);
                            print_request(session->client_req);
                        }

                        // --- Process parsed request -----
                        if(session->parsing_complete || session->parse_res != HTTP_OK){

                            // --- Setup -----
                            if(session->parsing_complete
                                 && strncmp(session->client_req->connection_type, "close", MAX_HEADER_VALUE_SIZE) == 0){
                                set_http_session_connection_type(session, CLOSE);
                                // keep-alive is set by default 
                            }

                            http_response_qnode_t *resp_node = http_resp_qnode_init();
                            if(!resp_node){
                                close_session = true; break;
                            }

                            // --- Route request -----
                            int route_res = route_request(cfg_infos, session, &resp_node->response);

                            if(route_res != 0){
                                http_resp_qnode_free(resp_node);
                                close_session = true; break;
                            }

                            // --- Send response -----
                            if(cfg_infos->verbose){
                                print_debug("Sending response : \n");
                                print_response(&resp_node->response);
                            }

                            bool send_successful = false; 

                            if(http_resp_queue_is_empty(session->resp_queue)){

                                if(send_http_resp_qnode(session->client_fd, resp_node) != 0){
                                    http_resp_qnode_free(resp_node);
                                    if(cfg_infos->verbose) print_error("Couldn't send http qnode\n");
                                    close_session = true; break;
                                }

                                send_successful = http_resp_qnode_is_sent(resp_node);
                            }

                            if(send_successful){
                                if(cfg_infos->verbose) print_debug("Successfully sent response\n");
                                http_resp_qnode_free(resp_node);
                            }
                            else{
                                if(http_resp_queue_add(session->resp_queue, resp_node) != 0){
                                    http_resp_qnode_free(resp_node);
                                    if(cfg_infos->verbose) print_error("Couldn't add http qnode to queue\n");
                                    close_session = true; break; 
                                }

                                evt->expect = EVT_WRITE;
                                if(evt_register(session->client_fd, evt, false) != 0){
                                    if(cfg_infos->verbose) print_error("Couldn't add client WRITE event to the event queue\n");
                                    close_session = true; break;
                                }
                            }

                            // Clean
                            if(reset_http_session_request_info(session) != 0){
                                close_session = true; break; 
                            }

                        } // done processing parsed request 

                    } // done reading client data

                    // Connection type is CLOSE 
                    if(close_session || session->connection_type == CLOSE){
                        if(cfg_infos->verbose && session->connection_type == CLOSE) print_debug("Connection type set to close\n");
                        close_http_session(session); continue;
                    }

                    evt->expect = EVT_READ; 
                    if(evt_register(session->client_fd, evt, false) != 0){
                        if(cfg_infos->verbose) print_error("Couldn't add client event to the event queue\n");
                        close_http_session(session); continue;
                    }

                    // Restart timeout timer
                    if(start_session_timer(session) != 0){
                        print_error("Server error: couldn't start client timeout timer\n");
                        close_http_session(session);
                    }
                }

                // Sendable data 
                if(evt->type == SOCKET_EVT && evt->expect & EVT_WRITE){
                    
                    if(send_http_resp_queue(session->client_fd, session->resp_queue) != 0){
                        close_http_session(session); continue;
                    }

                    if(!http_resp_queue_is_empty(session->resp_queue)){
                        evt->expect = EVT_WRITE; 
                        if(evt_register(session->client_fd, evt, false) != 0){
                            print_error("Server error: couldn't add client event to the event queue\n");
                            close_http_session(session); continue;
                        }
                    }
                }


                // Connection closed by client 
                if(evt->expect & EVT_CLOSE){
                    if(cfg_infos->verbose) print_info("Sever : connection closed by peer (fd : %d)\n", session->client_fd);
                    close_http_session(session);
                    continue;
                }


                // Timeout 
                if(evt->type == TIMER_EVT && evt->expect & EVT_TIMER){
                    if(cfg_infos->verbose) print_debug("Request timeout\n");
                    close_http_session(session);
                    continue;
                }

            } // end if server else client event  
        }
        

    } // Server event loop end


    close_http_session(server_session);
    close_evt_queue();
}
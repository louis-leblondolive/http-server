#include "listener.h"


void sigchld_handler(int s){
    (void)s; // silences unused variable warning 
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int send_all(int fd, char *buf, size_t len){

    size_t sent = 0;
    while(sent < len){
        ssize_t n = send(fd, (void*)buf + sent, len - sent, 0);
        if(n == -1) return -1;
        sent += n;
    }
    return 0;
}


void listener(config_infos *cfg_infos, int sock_fd){

    // listen on sock_fd, new connections on client_fd

    struct sigaction sa; 
    int client_fd;          
    struct sockaddr_storage client_addr; 
    socklen_t client_sin_size;


    if(listen(sock_fd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    if (!cfg_infos->quiet) printf("[INFO] Server online, listening on port %s\n", PORT);

    while(1){

        client_sin_size = sizeof(client_addr);
        client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_sin_size);
        if(client_fd == -1){
            perror("accept");
            continue;
        }
        
        if (!cfg_infos->quiet) printf("[INFO] Server : got connection from %s\n", sockaddr_in_addr_to_str(&client_addr));

        pid_t pid = fork();
        if(pid == -1){
            perror("server : fork");
            close(client_fd);
            continue;
        }

        if(pid == 0){    // child process
            close(sock_fd);

            // Receive data
            char raw_request[MAX_REQUEST_LEN];
            ssize_t bytes_received = recv(client_fd, raw_request, MAX_REQUEST_LEN, 0);

            if(bytes_received == 0){
                perror("server : peer closed its half side of the connection");
                close(client_fd);
                exit(1);
            }
            if(bytes_received == -1){
                perror("server : recv");
                close(client_fd);
                exit(1);
            }

            request client_req;
            memset(&client_req, 0, sizeof(request));
            http_status parse_res = parse_raw_request(raw_request, &client_req, bytes_received);
            
            // testing request
            if(!cfg_infos->quiet){
                printf("[INFO] parsed request : \n");
                print_request(&client_req);
            }
           

            // Process
            response serv_resp;
            memset(&serv_resp, 0, sizeof(response));
            if (route_request(cfg_infos, &client_req, &serv_resp, parse_res) != HTTP_OK){
                fprintf(stderr, "[ERROR] Server error while routing request\n");
                close(client_fd);
                exit(1);
            };
            
            // testing response
            if(!cfg_infos->quiet){
                printf("[INFO] processed request into response :\n");
                print_reponse(&serv_resp);
            }

            // Respond
            char *raw_response = build_text_response(cfg_infos, &serv_resp);
            if(!raw_response){
                fprintf(stderr, "[ERROR] Server couldn't build text response\n");
                close(client_fd);
                exit(1);
            }

            if(cfg_infos->verbose){
                printf("[DEBUG] sending raw response\n");
                printf("%s\n", raw_response);
            }

            int send_res = send_all(client_fd, raw_response, strlen(raw_response));
            
            if(!cfg_infos->quiet) printf("[INFO] closing connection\n");
            free(raw_response);
            close(client_fd);

            if(send_res == -1){
                perror("server : send");
                exit(1);
            }
            
            exit(0);
        }

        close(client_fd);
    }
}
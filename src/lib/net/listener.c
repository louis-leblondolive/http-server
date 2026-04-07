#include "listener.h"


void sigchld_handler(int s){
    (void)s; // silences unused variable warning 
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}


void listener(int sock_fd){

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

    printf("Server online, listening on port %s\n", PORT);

    while(1){

        client_sin_size = sizeof(client_addr);
        client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_sin_size);
        if(client_fd == -1){
            perror("accept");
            continue;
        }
        
        printf("server: got connection from %s\n", sockaddr_in_addr_to_str(&client_addr));

        if(fork() == 0){    // child process
            close(sock_fd);
            if(send(client_fd, "Hello, world !", 14, 0) == -1) perror("send");
            close(client_fd);
            exit(0);
        }

        close(client_fd);
    }
}
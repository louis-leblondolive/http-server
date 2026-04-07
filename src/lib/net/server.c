#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define PORT "3490"
#define BACKLOG 10


void sigchld_handler(int s){
    (void)s; // silences unused variable warning 
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family== AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void){

    struct sigaction sa; 
    struct addrinfo hints, *servinfo, *p; 
    int sock_fd, client_fd;                    // listen on sock_fd, new connections on client_fd
    struct sockaddr_storage client_addr; 
    socklen_t client_sin_size;
    int getaddr_rv;
    int yes = 1;

    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if ((getaddr_rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "%s\n", gai_strerror(getaddr_rv));
        return 1;
    }


    for (p = servinfo; p != NULL; p = p->ai_next){
        
        if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
            perror("setsockopt");
            freeaddrinfo(servinfo);
            exit(1);
        }

        if(bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server : bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL){
        fprintf(stderr, "server : failed to bind\n");
        exit(1);
    }

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

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if(fork() == 0){    // child process
            close(sock_fd);
            if(send(client_fd, "Hello, world !", 14, 0) == -1) perror("send");
            close(client_fd);
            exit(0);
        }

        close(client_fd);
    }
}
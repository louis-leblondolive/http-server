#include "setup.h"

int setup_server(void){

    int sock_fd;
    struct addrinfo hints, *servinfo, *p; 
    int getaddr_rv;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if ((getaddr_rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "%s\n", gai_strerror(getaddr_rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next){
        
        if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
            perror("setsockopt");
            freeaddrinfo(servinfo);
            return -1;
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
        return -1;
    }

    return sock_fd;
}
#include "setup.h"
#include "listener.h"


int main(void){

    int sock_fd = setup_server();
    if (sock_fd == -1){
        perror("server : socket setup error ");
        return 1;
    }

    listener(sock_fd);

    return 0;
}
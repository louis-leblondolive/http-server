#ifdef __APPLE__

#include "net_send.h"

#include <sys/socket.h>
#include <sys/uio.h>


ssize_t net_sendfile(int file_fd, int sock_fd, off_t offset, off_t len){

    off_t bytes_sent = len;
    int send_res = sendfile(file_fd, sock_fd, offset, &bytes_sent, NULL, 0);

    if(send_res == -1){
        if(errno == EAGAIN || errno == EINTR) return bytes_sent;
        else return send_res;
    }
    
    return (ssize_t)bytes_sent;
}


#endif // __APPLE__
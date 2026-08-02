#ifdef __linux__

#include "net_send.h"

#include <sys/sendfile.h>


ssize_t net_sendfile(int file_fd, int sock_fd, off_t offset, off_t len){

    off_t local_offset = offset; 
    ssize_t n_sent = sendfile(sock_fd, file_fd, &local_offset, (size_t)len);

    if(n_sent == -1){
        if(errno == EAGAIN || errno == EINTR) return 0;
        else return -1;
    }

    return n_sent;
}

#endif  //__linux__
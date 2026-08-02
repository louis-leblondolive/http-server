#ifndef NET_SEND
#define NET_SEND


#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

ssize_t net_sendfile(int file_fd, int sock_fd, off_t offset, off_t len);

#endif
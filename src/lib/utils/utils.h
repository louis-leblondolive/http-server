#ifndef UTILS
#define UTILS

#include <stdlib.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>


void *get_in_addr(struct sockaddr_storage *sa); 
// Returns the ip address associated to sa

char *sockaddr_in_addr_to_str(struct sockaddr_storage *sa); 
// Returns a string version of the ip address associated to sa 

#endif
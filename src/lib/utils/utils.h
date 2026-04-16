#ifndef UTILS
#define UTILS

#include <stdlib.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

/** 
 * @brief Returns the ip address associated to sa
 * @return Return value can either be caster to an IPv4 or IPv6 format
 * */

void *get_in_addr(struct sockaddr_storage *sa); 

/**
 *  @brief Returns a string version of the ip address associated to a 
 *  stored ip address 
 * */ 
char *sockaddr_in_addr_to_str(struct sockaddr_storage *sa); 

#endif
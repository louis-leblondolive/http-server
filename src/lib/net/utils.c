#include "utils.h"


void *get_in_addr(struct sockaddr_storage *sa){


    if (((struct sockaddr *)sa)->sa_family== AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


char *sockaddr_in_addr_to_str(struct sockaddr_storage *sa){

    char *s = (char*)malloc(sizeof(char) * INET6_ADDRSTRLEN);
    inet_ntop(sa->ss_family, get_in_addr(sa), s, sizeof s);
    return s;
}
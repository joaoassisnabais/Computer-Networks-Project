#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "tcp.h"


int serverTCP(void){

    struct addrinfo hints, *res;
    int fd, newfd, errcode;     ssize_t n,nw;
    char *ptr,buffer[128];

    if((fd==socket(AF_INET, SOCK_STREAM, 0) == -1)) exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET ;         //IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP socket
    hints.ai_flags = AI_PASSIVE;

    if((errcode=getaddrinfo(NULL, "58001", &hints, &res ))!= 0) exit(1);

    if(bind(fd, res->ai_addr,res->ai_addrlen) == -1) exit(1);

    if(listen(fd, 5) == -1) exit(1);

    return fd;
}

int accept_connectionTCP(int fd){

    struct sockaddr addr;
    socklen_t addrlen=sizeof(addr);
    int newfd=accept(fd, &addr, &addrlen);
    if(newfd==-1) exit(1);

    return newfd;
}

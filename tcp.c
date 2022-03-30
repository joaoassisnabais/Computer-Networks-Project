#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "tcp.h"
#include "node.h"

int clientTCP(char *serverIP, int serverPort){
    struct addrinfo hints, *res;
    int fd, n, errcode;
    char buffer[128], strPort[8];

    fd==socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1){perror("TCP client fd"); exit(1);}

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET ;         //IPv4
    hints.ai_socktype = SOCK_STREAM;    //TCP socket

    itoa(serverPort, strPort, 10);
    errcode=getaddrinfo(NULL, strPort, &hints, &res);
    if(errcode!=0){perror("TCP client getaddrinfo"); exit(1);}

    errcode=connect(fd, res->ai_addr, res->ai_addrlen);
    if(errcode==-1){perror("TCP client connect"); exit(1);}

    return fd;
}


int serverTCP(int port){
    struct addrinfo hints, *res;
    int fd, newfd, errcode;
    char buffer[128], strPort[8];

    if((fd==socket(AF_INET, SOCK_STREAM, 0) == -1)) exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET ;         //IPv4
    hints.ai_socktype = SOCK_STREAM;    //TCP socket
    hints.ai_flags = AI_PASSIVE;

    itoa(port, strPort, 10);
    if((errcode=getaddrinfo(NULL, strPort, &hints, &res))!= 0) exit(1);

    if(bind(fd, res->ai_addr,res->ai_addrlen) == -1) exit(1);

    if(listen(fd, 5) == -1) exit(1);

    return fd;
}

void closeServerTCP(){

}

void closeClientTCP(){

}


int accept_connectionTCP(int fd){

    struct sockaddr addr;
    socklen_t addrlen=sizeof(addr);
    int newfd=accept(fd, &addr, &addrlen);
    if(newfd==-1) exit(1);

    return newfd;
}

int treat_new_connectionTCP(int clientSocket, nodeState *state){
    char buffer[128];
    int nread=0;
    message msg;

    nread=read(clientSocket, &buffer, 128);
    sscanf(buffer,"%s %d %s %s", msg.command, msg.nodeKey, msg.ip, msg.port);
    strcpy(state->next->port, msg.port);
    strcpy(state->next->ip, msg.ip);
    state->next->key=msg.nodeKey;

    return 0;
}

void talkTCP(int fd, message *msg){
    int errcode=0;
    char str[128];

    sprintf(str, "%s %d %s %d\n", msg->command, msg->nodeKey, msg->ip, msg->port);
    errcode=write(fd,str,sizeof(str));
    if(errcode==-1){perror("write failed"); exit(1);}
    if(errcode<sizeof(str)){perror("write didn't write the whole message"); exit(1);}
}

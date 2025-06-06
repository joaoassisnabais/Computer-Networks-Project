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
    int fd, errcode;
    char strPort[32];

    fd=socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1){
        perror("TCP client fd");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET ;         //IPv4
    hints.ai_socktype = SOCK_STREAM;    //TCP socket

    sprintf(strPort,"%d",serverPort);
    errcode=getaddrinfo(serverIP, strPort, &hints, &res);
    if(errcode!=0){
        perror("TCP client getaddrinfo"); 
        exit(1);
    }

    errcode=connect(fd, res->ai_addr, res->ai_addrlen);
    if(errcode==-1){
        perror("TCP client connect"); 
        exit(1);
    }

    freeaddrinfo(res);

    return fd;
}


int serverTCP(char *IP, int port){
    struct addrinfo hints, *res;
    int fd, errcode;
    char strPort[8];

    if((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET ;         //IPv4
    hints.ai_socktype = SOCK_STREAM;    //TCP socket
    hints.ai_flags = AI_PASSIVE;

    sprintf(strPort,"%d",port);
    if((errcode=getaddrinfo(IP, strPort, &hints, &res))!= 0)
        exit(1);

    if(bind(fd, res->ai_addr,res->ai_addrlen) == -1)
        exit(1);

    if(listen(fd, 5) == -1)
        exit(1);

    freeaddrinfo(res);

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

/**
 * @brief Read tcp message incoming and place it in message struct
 * 
 * @param fd file descriptor
 * @param msg Message itself 
 * @return checks if session is closed
 */
int readTCP(int fd, message *msg){
    char buffer[128];
    int nread=0, trash;

    nread=read(fd, &buffer, 128);
    sscanf(buffer, "%s %d", msg->command, &trash);
    //session closed by the other end
    if(nread == 0){ 
        return -1;
    }
    else{
        if(strcmp(msg->command, "PRED") == 0 || strcmp(msg->command, "EPRED") == 0 || strcmp(msg->command, "SELF") == 0){
            sscanf(buffer,"%*s %d %s %d", &msg->nodeKey, msg->ip, &msg->port);   
        }
        else if(strcmp(msg->command, "EFND") == 0 || strcmp(msg->command, "FND") == 0 || strcmp(msg->command, "RSP") == 0){
            sscanf(buffer, "%*s %d %d %d %s %d", &msg->searchKey, &msg->sequenceN, &msg->nodeKey, msg->ip, &msg->port);
        }
        else{
            printf("\nIncoming TCP message command is not listed\n");
        }
        return 0;
    }
}

/**
 * @brief Receives a message and a file descriptor and writes to it
 * 
 * @param fd File descriptor to write to
 * @param msg Message to send
 */
void talkTCP(int fd, message *msg){
    //PROTECT AGAINST SIGPIPE
    int errcode=0;
    char buffer[128];

    if(strcmp(msg->command, "PRED") == 0 || strcmp(msg->command, "EPRED") == 0 || strcmp(msg->command, "SELF") == 0){
        sprintf(buffer, "%s %d %s %d\n", msg->command, msg->nodeKey, msg->ip, msg->port);
        errcode=write(fd,buffer,sizeof(buffer));
    }
    else if(strcmp(msg->command, "FND") == 0 || strcmp(msg->command, "RSP") == 0){
        sprintf(buffer, "%s %d %d %d %s %d\n", msg->command, msg->searchKey, msg->sequenceN, msg->nodeKey, msg->ip, msg->port);
        errcode=write(fd,buffer,sizeof(buffer));
    }
    else{
        printf("\nOutgoing message command is not listed\n");
    }

    if(errcode==-1){
        perror("write failed");
        exit(1);
    }
    if(errcode<sizeof(buffer)){
        perror("write didn't write the whole message"); 
        exit(1);
    }

}

/**
 * @brief Closes a TCP connection and checks for errors
 * 
 * @param fd Socket of the TCP connection
 */
void closeTCP(int fd){
    if(close(fd) == -1){
        perror("Close TCP session failed");
        exit(1);
    }
}
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "udp.h"
#include "tcp.h"
#include "node.h"

int clientTalkUDP(char *serverIP, int serverPort, message *msg){

    struct addrinfo hints, *res;    //create addrinfo type structs to store hints and response data
    int errcode, n;
    char strPort[32], strBuffer[128];

    memset(&hints, 0, sizeof(hints));   //inits hints
    hints.ai_family = AF_INET;  //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    sprintf(strPort, "%d", serverPort);
    errcode = getaddrinfo(serverIP, strPort, &hints, &res);
    if(errcode != 0) exit(1);

    if(strcmp(msg->command, "FND")==0 || strcmp(msg->command, "RSP")==0)
        sprintf(strBuffer, "%s %d %d %d %s %d", msg->command, msg->searchKey, msg->sequenceN, msg->nodeKey, msg->ip, msg->port);
    
    if(strcmp(msg->command, "EFND")==0)
        sprintf(strBuffer, "%s %d", msg->command, msg->nodeKey);

    if(strcmp(msg->command, "EPRED")==0)
        sprintf(strBuffer, "%s %d %s %d", msg->command, msg->nodeKey, msg->ip, msg->port);

    n = sendto(serverSocketUDP, strBuffer, strlen(strBuffer), 0,res->ai_addr, res->ai_addrlen);

    if(n == -1){
        perror("UDP write failed");
        exit(1);
    }

    fd_set socketUDP;
    struct timeval tv;
    bool recAck=false;
    message ack;
    FD_ZERO(&socketUDP);
    FD_SET(serverSocketUDP, &socketUDP);

    tv.tv_sec=1;
    //do 3 iterations (resends) expecting for ack
    for(int i=0; i<3; i++){
        if(select(serverSocketUDP+1, &socketUDP, NULL, NULL, &tv) < 0){
            perror("UDP select");
            exit(1);
        }

        if(FD_ISSET(serverSocketUDP, &socketUDP)){
            receive_messageUDP(serverSocketUDP, &ack);
            recAck=true;
            break;  //to exit if an ack is received
        }

        n = sendto(serverSocketUDP, strBuffer, strlen(strBuffer), 0,res->ai_addr, res->ai_addrlen);

        if(n == -1){
        perror("UDP write failed");
        exit(1);
    }
    }

    //see if i received an ack
    if(!recAck){
        //see if the message can be sent via TCP
        if(strcmp(msg->command, "FND")==0 || strcmp(msg->command, "RSP")==0){
            talkTCP(serverSocketTCP, msg);
        }
        else if(strcmp(msg->command, "EPRED")==0 || strcmp(msg->command, "EFND")==0){
            printf("\nDidn't receive in response to %s message\n", msg->command);
            printf("Gave up on connection\n");
        }
    }


    freeaddrinfo(res);
    return 0;
}

int serverUDP(char *IP, int port){

    struct addrinfo hints,*res;
    int fd, errcode;
    char strPort[32];

    fd=socket(AF_INET,SOCK_DGRAM,0); //open UDP socket
    if(fd==-1) exit(1); //error check
    
    memset(&hints, 0, sizeof(hints));   //inits hints
    hints.ai_family= AF_INET ;  //IPv4
    hints.ai_socktype= SOCK_DGRAM; //UDP socket
    hints.ai_flags=AI_PASSIVE;

    sprintf(strPort, "%d", port);
    errcode=getaddrinfo(IP, strPort, &hints, &res);
    if(errcode!=0) exit(1); //error check

    errcode=bind(fd, res->ai_addr, res->ai_addrlen);
    if(errcode==-1) exit(1); //error check

    freeaddrinfo(res);
    
    return fd;
}

void receive_messageUDP(int serverSocket, message *msg){
 
    ssize_t nread;
    char buffer[128+1];
    struct sockaddr addr;
    socklen_t addrlen;

    nread=recvfrom(serverSocket, buffer, 128, 0, &addr, &addrlen);
    sscanf(buffer, "%s %*d", msg->command);
    
    if(nread==-1) {
        perror("UDP read error");
        exit(1);
    }

    if(strcmp(msg->command, "ACK") == 0){
        printf("received ack");
        return;
    }
    else if(strcmp(msg->command, "EPRED") == 0){
        sscanf(buffer,"%*s %d %s %d", &msg->nodeKey, msg->ip, &msg->port);   
    }
    else if(strcmp(msg->command, "EFND") == 0 || strcmp(msg->command, "FND") == 0 || strcmp(msg->command, "RSP") == 0){
        sscanf(buffer, "%*s %d %d %d %s %d", &msg->searchKey, &msg->sequenceN, &msg->nodeKey, msg->ip, &msg->port);
    }
    else{
        printf("\nIncoming UDP message command is not listed\n");
        return;
    }

    char str[5];
    strcpy(str, "ACK");    
    int n = sendto(serverSocket, str, strlen(str), 0, &addr, addrlen);

    if(n == -1){
        perror("UDP write failed");
        exit(1);
    }

}
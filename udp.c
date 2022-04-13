#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "udp.h"
#include "node.h"

void sendACK(){
    message aux;
    strcpy(aux.command, "ACK");

    clientTalkUDP(NULL, -1, &aux);
}

int clientTalkUDP(char *serverIP, int serverPort, message *msg){

    struct addrinfo hints, *res;    //create addrinfo type structs to store hints and response data
    int fd, errcode, n;
    char strPort[32], strBuffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0);    //open UDP socket
    if(fd == -1) exit(1);

    memset(&hints, 0, sizeof(hints));   //inits hints
    hints.ai_family = AF_INET;  //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    if(serverIP != NULL){   //
        sprintf(strPort, "%d", serverPort);
        errcode = getaddrinfo(serverIP, strPort, &hints, &res);
        if(errcode != 0) exit(1);
        flagACK = 1;    //expect an ACK after sending UDP msg
    }
    else if(strcmp(msg->command, "ACK")==0) {
        *(res->ai_addr) = addrACKSendUDP;
        res->ai_addrlen = addrlenACKSendUDP;

        sprintf(strBuffer, "%s", msg->command);
        flagACK = 0;    //don't expect ACK after sending ACK
    }
    else{
        *(res->ai_addr) = addrResendUDP;
        res->ai_addrlen = addrlenResendUDP;
        flagACK = 1;    //expect an ACK after sending UDP msg
    }


    if(strcmp(msg->command, "FND")==0 || strcmp(msg->command, "RSP")==0)
        sprintf(strBuffer, "%s %d %d %d %s %d", msg->command, msg->searchKey, msg->sequenceN, msg->nodeKey, msg->ip, msg->port);
    
    
    else if(strcmp(msg->command, "EFND")==0)
        sprintf(strBuffer, "%s %d", msg->command, msg->nodeKey);

    else if(strcmp(msg->command, "EPRED")==0)
        sprintf(strBuffer, "%s %d %s %d", msg->command, msg->nodeKey, msg->ip, msg->port);

    n = sendto(fd, strBuffer, strlen(strBuffer), 0,res->ai_addr, res->ai_addrlen);

    if(flagACK){    //message to resend if ACK isn't received
        strcpy(msgResendUDP.command, msg->command);
        msgResendUDP.nodeKey = msg->nodeKey;
        msgResendUDP.sequenceN = msg->sequenceN;
        msgResendUDP.searchKey = msg->searchKey;
        strcpy(msgResendUDP.ip, msg->ip);
        msgResendUDP.port = msg->port;
    }

    if(n == -1){
        perror("UDP write failed");
        exit(1);
    }

    freeaddrinfo(res);
    close(fd);

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

    nread=recvfrom(serverSocket, buffer, 128, 0, &addrACKSendUDP, &addrlenACKSendUDP);    //store addr info to send ACKs and answer EPRED to New Node once its place is found
    sscanf(buffer, "%s %*d", msg->command);
    
    if(nread==-1) {
        perror("UDP read error");
        exit(1);
    }

    if(strcmp(msg->command, "ACK") == 0){
        flagACK = 0;    //ACK received, turn off flag
        return;
    }
    else if(strcmp(msg->command, "EPRED") == 0){
        sscanf(buffer,"%*s %d %s %d", &msg->nodeKey, msg->ip, &msg->port);
        addrNewNode = addrACKSendUDP;
        addrlenNewNode = addrlenACKSendUDP;
    }
    else if(strcmp(msg->command, "EFND") == 0 || strcmp(msg->command, "FND") == 0 || strcmp(msg->command, "RSP") == 0){
        sscanf(buffer, "%*s %d %d %d %s %d", &msg->searchKey, &msg->sequenceN, &msg->nodeKey, msg->ip, &msg->port);
    }
    else{
        printf("\nIncoming message command is not listed\n");
    }

    sendACK();  //if msg received isn't ACK, send ACK
}
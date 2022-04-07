#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int clientUDP(char **hostname, char **port){

    struct addrinfo hints, *res;    //create addrinfo type structs to store hints and response data
    int fd, errcode;

    fd = socket(AF_INET, SOCK_DGRAM, 0);    //open UDP socket
    if(fd == -1) exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(*hostname, *port, &hints, &res);
    if(errcode != 0) exit(1);

    freeaddrinfo(res);

    return 0;
}

int serverUDP(void){

    struct addrinfo hints,*res;
    int fd, errcode;

    fd=socket(AF_INET,SOCK_DGRAM,0); //open UDP socket
    if(fd==-1) exit(1); //error check
    
    memset(&hints, 0, sizeof(hints));   //sets hints in memory
    hints.ai_family= AF_INET ;  //IPv4
    hints.ai_socktype= SOCK_DGRAM; //UDP socket
    hints.ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(NULL, "58001", &hints, &res);    //CHANGE 58001->PORT
    if(errcode!=0) exit(1); //error check

    errcode=bind(fd, res->ai_addr, res->ai_addrlen);
    if(errcode==-1) exit(1); //error check

    freeaddrinfo(res);
    
    return fd;
}

int receive_messageUDP(int serverSocket, char *buffer){
 
    struct sockaddr addr;
    socklen_t addrlen;
    ssize_t nread;

    nread=recvfrom(serverSocket, buffer, 128, 0, &addr, &addrlen);

    return nread;
}
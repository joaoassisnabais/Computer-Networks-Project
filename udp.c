#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#define LOCAL_PORT "58001"

int clientUDP(char **hostname, char **port){

    struct addrinfo hints, *res;    //create addrinfo type structs to store hints and response data
    int fd, errcode;
    ssize_t n;

    fd = socket(AF_INET, SOCK_DGRAM, 0);    //open UDP socket
    if(fd == -1) exit(1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket

    errcode = getaddrinfo(*hostname, *port, &hints, &res);
    if(errcode != 0) exit(1);
}

int serverUDP(void){

    struct addrinfo hints,*res;
    int fd, errcode;
    struct sockaddr addr;
    socklen_t addrlen;
    ssize_t n, nread;
    char buffer[128];

    fd=socket(AF_INET,SOCK_DGRAM,0); //open UDP socket
    if(fd==-1) exit(1); //error check
    
    memset(&hints, 0, sizeof(hints));   //sets hints in memory
    hints.ai_family= AF_INET ;  //IPv4
    hints.ai_socktype= SOCK_DGRAM; //UDP socket
    hints.ai_flags=AI_PASSIVE;

    errcode=getaddrinfo(NULL, LOCAL_PORT, &hints, res);
    if(errcode!=0) exit(1); //error check

    errcode=bind(fd, res->ai_addr, res->ai_addrlen);
    if(errcode==-1) exit(1); //error check
    
    /*FALTA DEVOLVER ADDRESS IF I'M NOT MISTAKEN*/
    
    return 0;
}
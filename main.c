#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "tcp.h"
#include "udp.h"

#define max(A,B) ((A)>=(B)?(A):(B))

//NEED TO PROTECT AGAINST SIGPIPE

int main(int argc, char **argv){

    char buffer[128], option[7];
    fd_set currentSockets, readySockets;
    int serverSocketTCP=serverTCP(), serverSocketUDP=serverUDP(), maxfd;

    //init current set
    FD_ZERO(&currentSockets);
    FD_SET(serverSocketTCP, &currentSockets);
    FD_SET(0,&currentSockets);
    maxfd=max(serverSocketTCP,serverSocketUDP);

    while(1){
        //select is destructive
        readySockets=currentSockets;

        if(select(maxfd+1, &readySockets, NULL, NULL, NULL) < 0) exit(1);

        for(int i=0; i < maxfd; i++){
            //checking what sockets are set
            if(FD_ISSET(i,&readySockets)){
                //detecting stdin input
                if(i==0){
                    FD_CLR(0,&readySockets);    //DON'T KNOW WHY IT'S NEEDED
                    strcpy(buffer,"");
                    fgets(buffer, 128, stdin);
                    if(sscanf(buffer,"%s", option) != 1) exit(1);
                    
                    if(strcmp(option,"exit") == 0 || strcmp(option,"e") == 0){
                        printf("User input for exit\n Exiting...\n");
                        exit(0);
                    }
                    else if(strcmp(option,"new") == 0 || strcmp(option,"n") == 0 ){
                        

                    }
                    else if(strcmp(option,"bentry") == 0 || strcmp(option,"b") == 0){


                    }
                    else if(strcmp(option,"pentry") == 0 || strcmp(option,"p") == 0){


                    }
                    else if(strcmp(option,"chord") == 0 || strcmp(option,"c") == 0){


                    }
                    else if(strcmp(option,"dchord") == 0 || strcmp(option,"echord") == 0 || strcmp(option,"d")){


                    }
                    else if(strcmp(option,"show") == 0 || strcmp(option,"s") == 0){


                    }
                    else if(strcmp(option,"find") == 0 || strcmp(option,"f") == 0){


                    }
                    else if(strcmp(option,"leave") == 0 || strcmp(option,"l") == 0){


                    }
                }
                //new connection to tcp server
                if(i==serverSocketTCP){
                    int clientSocketTCP=accept_connectionTCP(serverSocketTCP);
                    FD_SET(clientSocketTCP, &currentSockets);
                    
                    //CREATE A BACKWARDS SESSION WITH CLIENT FOR CONTROL PURPOSES

                    maxfd=max(clientSocketTCP,maxfd);
                }
                if(i==serverSocketUDP){
                    //recvfrom()
                    //do what its supposed to do with the connection
                    //basically turn on the read but maybe doing it by parts so it doesn't accept every packet at once
                    FD_CLR(i, &currentSockets); //clearing the socket from current sockets

                }
            }
        }
    }
    return 0;
}
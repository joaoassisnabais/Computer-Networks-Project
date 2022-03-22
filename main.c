#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "tcp.h"

#define max(A,B) ((A)>=(B)?(A):(B))

//NEED TO PROTECT AGAINST SIGPIPE

int main(int argc, char **argv){

    char buffer[100];

    //holding the program until "exit" user input
    while(1){
        gets(buffer);
        if(buffer==NULL){
            printf("No input arguments or too many input arguments");
            exit(1);
        }

        if(argc=0){
            printf("Not enough arguments");
            exit(1);
        }else if(strcmp(argv[0],"exit") == 0){
            printf("exit input by user");   
            exit(0);
        }


        int serverSocket=serverTCP();
        fd_set currentSockets, readySockets;

        //init current set
        FD_ZERO(&currentSockets);
        FD_SET(serverSocket, &currentSockets);

        while(1){
            //select is destructive
            readySockets=currentSockets;

            if(select(FD_SETSIZE, &readySockets, NULL, NULL, NULL) < 0){
                exit(1);
            };

            for(int i=0; i < FD_SETSIZE; i++){

                if(FD_ISSET(i,&readySockets)){
                    //new connection to tcp server
                    if(i==serverSocket){
                        int clientSocket=accept_connection(serverSocket);
                        FD_SET(clientSocket, &currentSockets);
                    }else{
                        //do what its supposed to do with the connection
                        //basically turn on the read but maybe doing it by parts so it doesn't accept every packet at once
                        FD_CLR(i, &currentSockets); //clearing the socket from current sockets

                    }
                }
            }
        }
    }
    return 0;
}
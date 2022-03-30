#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "udp.h"
#include "tcp.h"
#include "calls.h"
#include "node.h"

#define max(A,B) ((A)>=(B)?(A):(B))

void change_prev(nodeState *currNode, nodeInfo newNode){
    strcmp(currNode->next->ip, newNode.ip);
    currNode->next->key = newNode.key;
    currNode->next->port = newNode.port;
}

/**
 * @brief Initialize UDP and TCP servers
 * 
 * @param current current fd_set
 * @param TCP TCP file descriptor
 * @param UDP UDP file descriptor
 * @param maxfd maximum file descriptor so far
 * @param port port to open up the servers
 * @return maxfd
 */
int initServers(fd_set *current, int *TCP, int *UDP, int maxfd, int port){
    *TCP=serverTCP(port);
    *UDP=serverUDP();
    FD_SET(*TCP, current);
    FD_SET(*UDP, current);
    maxfd=max(maxfd, *TCP);
    maxfd=max(maxfd,*UDP);
    return maxfd;
}

/**
 * @brief initialize next and previous nodes into state variables
 * 
 * @param isNew indicates if it's a new ring or not
 * @param state state variables
 * @param prev struct with variables from prev node
 * @param next struct with variables from next node 
 */
void initState(bool isNew, nodeState *state, nodeInfo *prev, nodeInfo *next){
    state->next=(nodeInfo*)malloc(sizeof(nodeInfo));
    state->prev=(nodeInfo*)malloc(sizeof(nodeInfo));
    if(isNew==1){
        //define next and prev equal to self
        state->next->key=state->self->key;
        state->next->port=state->self->port;
        strcpy(state->next->ip, state->self->ip);
        state->prev->key=state->self->key;
        state->prev->port=state->self->port;
        strcpy(state->prev->ip, state->self->ip);
    }else{
        if(prev!=NULL){
            state->next->key=next->key;
            state->next->port=next->port;
            strcpy(state->next->ip, next->ip);      
        }
        if(next!=NULL){
            state->prev->key=prev->key;
            state->prev->port=prev->port;
            strcpy(state->prev->ip, prev->ip);
        }
    }
}

/**
 * @brief Initialize self into state variables and allocate memory
 * 
 * @param k self key
 * @param ip self ip
 * @param port self port
 * @param state struct with state variables
 */
void initSelf(int k, char *ip, int port, nodeState *state){
    state=(nodeState*)malloc(sizeof(nodeState));
    state->self=(nodeInfo*)malloc(sizeof(nodeInfo));
    state->self->key=k;
    state->self->port=port;
    strcpy(state->self->ip, ip);
}

/**
 * @brief Exit->free all state vars | Leave->frees next and prev
 * 
 * @param state State Variables
 * @param isLeave Is or not coming from leave command
 */
void closeSelf(nodeState *state, bool isLeave){
    if(!isLeave){
        free(state->self);
        free(state);
    }
    free(state->next);
    free(state->prev);
}


void core(int selfKey, char *selfIP, int selfPort){
    char buffer[128], option[7];
    fd_set currentSockets, readySockets;
    int serverSocketTCP=0, nextSocket, prevSocket;
    int serverSocketUDP, maxfd;
    bool isNew;
    nodeState *state;

    initSelf(selfKey, selfIP, selfPort, state);

    //init current set
    FD_ZERO(&currentSockets);
    FD_SET(0,&currentSockets);
    maxfd=0;

    while(1){
        readySockets=currentSockets;

        if(select(maxfd+1, &readySockets, NULL, NULL, NULL) < 0) exit(1);

        //checking what sockets are set
        if(FD_ISSET(0,&readySockets)){
            //detecting stdin input
            strcpy(buffer,"");
            fgets(buffer, 128, stdin);
            if(sscanf(buffer,"%s", option) != 1) exit(1);
            
            if(strcmp(option,"exit") == 0 || strcmp(option,"e") == 0){
                printf("User input for exit\n Exiting...\n");
                closeSelf(state,0);
                exit(0);
            }

            else if(strcmp(option,"new") == 0 || strcmp(option,"n") == 0 ){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, selfPort);
                initState(1, &state, NULL, NULL);
            }

            else if(strcmp(option,"bentry") == 0 || strcmp(option,"b") == 0){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, selfPort);
                initState(0, &state, NULL, NULL);   //FALTA fazer isto preciso fazer find primeiro
            }

            else if(strcmp(option,"pentry") == 0 || strcmp(option,"p") == 0){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, selfPort);
                prevSocket=pentry(state, buffer);
            }

            else if(strcmp(option,"chord") == 0 || strcmp(option,"c") == 0){
                if(maxfd==0) exit(1);
                
            }

            else if(strcmp(option,"dchord") == 0 || strcmp(option,"echord") == 0 || strcmp(option,"d")){
                if(maxfd==0) exit(1);

            }

            else if(strcmp(option,"show") == 0 || strcmp(option,"s") == 0){
                if(maxfd==0) exit(1);
            
            }

            else if(strcmp(option,"find") == 0 || strcmp(option,"f") == 0){
                if(maxfd==0) exit(1);

            }

            else if(strcmp(option,"leave") == 0 || strcmp(option,"l") == 0){
                if(maxfd==0) printf("Already out of any ring");
                else closeSelf(state, 1);
                

            }
        }
        //new connection to tcp server
        if(FD_ISSET(serverSocketTCP,&readySockets)){
            nextSocket=accept_connectionTCP(serverSocketTCP);
            FD_SET(nextSocket, &currentSockets);
            //need to write the rest -> basically if there is a next already close the channel and advise them of new prev
            // if there isnt any just accept and create state vars 


            maxfd=max(nextSocket,maxfd);
        }
        if(FD_ISSET(serverSocketUDP,&readySockets)){
            //recvfrom()
            //do what its supposed to do with the connection
            //basically turn on the read but maybe doing it by parts so it doesn't accept every packet at once
            

        }
        if((FD_ISSET(prevSocket,&readySockets))){

            

        }
    }


}
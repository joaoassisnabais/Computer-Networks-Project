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

//sequence numbers and find Index are global variables
int seq[100], findI;
//address of query node (after EFND)
struct sockaddr addr;
socklen_t addrlen;

/**
 * @brief Finds distance between two keys in the ring
 * 
 * @param start Starting point 
 * @param end End point
 * @return Result 
 */
int dist(int start, int end){
    int res;
    res=(end-start)%32;
    if (res<0) res+=32;
    
    return res;
}

/**
 * @brief Initializes sequence with -1 in every index
 * 
 * @param seq Sequence number vector
 */
void initSeq(void){
    for(int i=0; i<100; i++) seq[i]=-1;
}

/**
 * @brief Prints the user command menu
 * 
 */
void printmenu(bool isFull){
    printf("\n#################################################");

    if(isFull){
        printf("\n\nThis is the application menu. Input 'menu' or 'm' to see it again.");
        printf("\n\nnew\n\t-> Cria um anel contendo apenas o nó");
        printf("\n\nbentry boot boot.IP boot.port\n\t");
        printf("-> Entrada do nó no anel ao qual pertence o nó boot com endereço IP boot.IP e porto boot.port");
        printf("\n\npentry pred pred.IP pred.port\n\t");
        printf("-> Entrada do nó no anel sabendo que o seu predecessor será o nó pred com endereço IP pred.IP e porto pred.port");
        printf("\n\nchord i i.IP i.port\n\t");
        printf("-> Cria um atalho para o nó i com endereço IP i.IP e porto i.port");
        printf("\n\nechord\n\t");
        printf("-> Elimina o atalho");
        printf("\n\nshow\n\t");
        printf("-> Mostra o estado do nó");
        printf("\n\nfind k\n\t");
        printf("-> Procura a chave k, retornando a mesma, o endereço IP e o porto do nó à qual a chave pertence");
        printf("\n\nleave\n\t");
        printf("-> Saída do nó do anel");
        printf("\n\nexit\n\t");
        printf("-> Fecho do programa\n");
    }
    printf("\n#################################################\n");   
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
int initServers(fd_set *current, int *TCP, int *UDP, int maxfd, nodeState *state){
    *TCP=serverTCP(state->self->ip, state->self->port);
    *UDP=serverUDP(state->self->ip, state->self->port);
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
 * @param pfd predecessors file descriptor
 * @param nfd next file descriptor
 */
void initState(bool isNew, nodeState *state, nodeInfo *prev, nodeInfo *next, int pfd, int nfd){
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
            state->prev->key=prev->key;
            state->prev->port=prev->port;
            strcpy(state->prev->ip, prev->ip);
            state->prev->fd=pfd;      
        }
        if(next!=NULL){
            state->next->key=next->key;
            state->next->port=next->port;
            strcpy(state->next->ip, next->ip);
            state->next->fd=nfd;
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
void initSelf(int k, char *ip, int port, nodeState **state){
    *state=(nodeState*)malloc(sizeof(nodeState));
    (*state)->self=(nodeInfo*)malloc(sizeof(nodeInfo));
    (*state)->old=(nodeInfo*)malloc(sizeof(nodeInfo));
    (*state)->next=(nodeInfo*)malloc(sizeof(nodeInfo));
    (*state)->prev=(nodeInfo*)malloc(sizeof(nodeInfo));
    (*state)->SC=(nodeInfo*)malloc(sizeof(nodeInfo));
    (*state)->self->key=k;
    (*state)->self->port=port;
    strcpy((*state)->self->ip, ip);
    (*state)->old->fd=-1;   //init with no old socket
    (*state)->SC->fd=-1;    //fd is only used to determine if a shortcut exists (0 for yes, -1 for no)
    (*state)->next->fd=-1;
    (*state)->prev->fd=-1;
}

/**
 * @brief Exit->free all state vars | Leave->frees next and prev
 * 
 * @param state State Variables
 * @param isLeave Is or not coming from leave command
 */
void freeSelf(nodeState *state){
    free(state->self);
    free(state->old);
    free(state->next);
    free(state->prev);
    free(state->SC);
    free(state);
}

/** */
void initSC(nodeState *state, char *buffer) {
    int key, port;
    char ip[16];

    sscanf(buffer, "%*s %d %s %d", &key, ip, &port);

    state->SC->key=key;
    strcpy(state->SC->ip, ip);
    state->SC->port=port;
    state->SC->fd=0;    //fd is only used to determine if a shortcut exists (0 for yes, -1 for no)
}

/** */
void closeSC(nodeState *state){
    state->SC->fd=-1;   //fd set to -1 to indicate shortcut doesn't exist anymore
}

/**
 * @brief Core function that is always running
 * 
 * @param selfKey My key
 * @param selfIP My IP
 * @param selfPort My port
 */
void core(int selfKey, char *selfIP, int selfPort){
    char buffer[128], option[7];
    fd_set currentSockets, readySockets;
    int serverSocketTCP, serverSocketUDP, maxfd, errcode;
    nodeState *state = NULL;
    message msg;

    findI=-1;
    initSeq();
    initSelf(selfKey, selfIP, selfPort, &state);
    printmenu(1);

    //initialize current set
    FD_ZERO(&currentSockets);
    FD_SET(0,&currentSockets);
    maxfd=0;
    
    while(1){
        FD_ZERO(&readySockets);
        memcpy(&readySockets,&currentSockets,sizeof(currentSockets));
        
        if(select(maxfd+1, &readySockets, NULL, NULL, NULL) < 0) exit(1);

        //checking what sockets are set
        if(FD_ISSET(0,&readySockets)){
            //detecting stdin input
            strcpy(buffer,"");
            fgets(buffer, 128, stdin);
            if(sscanf(buffer,"%s", option) != 1) exit(1);

            if(strcmp(option,"new") == 0 || strcmp(option,"n") == 0 ){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, state);
                initState(1, state, NULL, NULL, -1, -1);
            }

            else if(strcmp(option,"bentry") == 0 || strcmp(option,"b") == 0){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, state);
                initState(0, state, NULL,  NULL, -1, -1);   //FALTA fazer isto preciso fazer find primeiro
            }

            else if(strcmp(option,"pentry") == 0 || strcmp(option,"p") == 0){
                maxfd=initServers(&currentSockets, &serverSocketTCP, &serverSocketUDP, maxfd, state);
                pentry(state, buffer);
                FD_SET(state->prev->fd, &currentSockets);
                maxfd=max(state->prev->fd,  maxfd);
            }

            else if(strcmp(option,"chord") == 0 || strcmp(option,"c") == 0){
                if(maxfd==0) 
                    printf("Can't be done outside of a ring\n"); 
                initSC(state, buffer);
            }

            else if(strcmp(option,"dchord") == 0 || strcmp(option,"echord") == 0 || strcmp(option,"d") == 0){
                if(maxfd==0) 
                    printf("Can't be done outside of a ring\n");   //cant be done without initialized ring
                closeSC(state);
            }

            else if(strcmp(option,"show") == 0 || strcmp(option,"s") == 0){
                if(maxfd==0) 
                    printf("Can't be done outside of a ring\n");   //cant be done without initialized ring
                show(state);
            }

            else if(strcmp(option,"find") == 0 || strcmp(option,"f") == 0){
                if(maxfd==0) 
                    printf("Can't be done outside of a ring\n");   //cant be done without initialized ring
                find(state, buffer, NULL);
            }

            else if(strcmp(option,"leave") == 0 || strcmp(option,"l") == 0){
                if(maxfd==0) 
                    printf("\nNot in any ring\n");
                else
                    leave(state, &currentSockets, &maxfd, serverSocketTCP, serverSocketUDP);
            }

            else if(strcmp(option,"exit") == 0 || strcmp(option,"e") == 0){
                if(maxfd!=0)
                    leave(state, &currentSockets, &maxfd, serverSocketTCP, serverSocketUDP);

                printf("\nExiting...\n");
                freeSelf(state);
                exit(0);
            }

            else if(strcmp(option, "menu") == 0 || strcmp(option, "m") == 0){
                printmenu(1);
            }

            else{
                printf("\nNot a valid option, please try again\n");
            }

            //prints divider between inputs
            printmenu(0);
        }
        //new connection to tcp server
        if(FD_ISSET(serverSocketTCP,&readySockets)){
            if(state->next->fd != -1){
                state->old->fd= state->next->fd;
                state->old->key=state->next->key;
                state->old->port=state->next->port;
                strcpy(state->old->ip, state->next->ip);
            }
            state->next->fd=accept_connectionTCP(serverSocketTCP);
            FD_SET(state->next->fd, &currentSockets);
            maxfd=max(state->next->fd,maxfd);
        }
        if(FD_ISSET(serverSocketUDP, &readySockets)){
            receive_messageUDP(serverSocketUDP, &msg);
            rcv_msg(&msg, state, &currentSockets, &maxfd);
            //do what its supposed to do with the connection
            

        }
        if(FD_ISSET(state->prev->fd, &readySockets)){
            errcode = readTCP(state->prev->fd, &msg);
            
            //Other end has closed the session
            if(errcode == -1){
                closeTCP(state->prev->fd);
                FD_CLR(state->prev->fd, &currentSockets);
                state->prev->fd=-1;
            }else{
                rcv_msg(&msg, state, &currentSockets, &maxfd);
            }
            
        }
        if(FD_ISSET(state->next->fd, &readySockets)){
            errcode = readTCP(state->next->fd, &msg);
            
            //Other end has closed the session
            if(errcode == -1){
                closeTCP(state->next->fd);
                FD_CLR(state->next->fd, &currentSockets);
                state->next->fd=-1;
            }else{
                rcv_msg(&msg, state, &currentSockets, &maxfd);
            }
        }
        if(FD_ISSET(state->old->fd, &readySockets)){
            errcode = readTCP(state->old->fd, &msg);
            //this means other end has closed the session
            if(errcode == -1){
                closeTCP(state->old->fd);
                FD_CLR(state->old->fd, &currentSockets);
                state->old->fd = -1;
            }else{
                perror("Message from leaving node is not FIN");
                exit(1);
            }
        }
    }    
}
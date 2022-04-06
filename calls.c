#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "node.h"
#include "udp.h"
#include "tcp.h"
#include "calls.h"

#define dist(origin,key) ((key-origin)%32)


/**
 * @brief Receives a message and decides what to do with it (message control)
 * 
 * @param msg  The message itself
 * @param state State Variables
 * @param current Current fd_set so it can be updated
 */
void rcv_msg(message *msg, nodeState *state, fd_set *current){

    if(strcmp(msg->command, "SELF") == 0){
        nodeInfo next;
        next.key=msg->nodeKey;
        next.port=msg->port;
        strcpy(next.ip, msg->ip); 
        initState(0, state, NULL, &next, -1, state->next->fd); //redundancy, there would be no need tp send fd but to have a complete function sent it anyway
        
        //if an old socket exists check what's closer to see if it's a node leaving or a new node entering
        if(state->old->fd != -1){
            if(dist(state->old->key,state->self->key) > dist(state->next->key, state->self->key)){ 
                msgPred(state->old->fd, state->next);
            }
        }else{
            strcpy(msg->command,"PRED");
        }
    }

    if(strcmp(msg->command, "PRED") == 0){
        nodeInfo prev;

        if(state->prev->fd != -1){
            closeTCP(state->prev->fd);
            FD_CLR(state->prev->fd, current);
            state->prev->fd=-1;
        }

        prev.key=msg->nodeKey;
        prev.port=msg->port;
        strcpy(prev.ip, msg->ip);

        prev.fd = clientTCP(&msg->ip, msg->port);
        initState(0, &state, &prev, NULL, prev.fd, -1);
        FD_SET(prev.fd, current);
        msgSelf(prev.fd, state->self);
    }

    if(strcmp(msg->command, "RSP") == 0){


    }
}

/**
 * @brief Entry in the ring knowing the predecessor
 * 
 * @param state State Variables
 * @param info String with the predecessors variables
 */
void pentry(nodeState *state, char *info){
    char* trash;
    nodeInfo prev, next;
    int prevSocket;

    if(info != NULL){
        sscanf(info, "%s %d %s %d", trash, prev.key, prev.ip, prev.port);
    }
    prev.fd = clientTCP(&prev.ip, prev.port);
    
    initState(0, &state, &prev, NULL, prev.fd, -1);
    msgSelf(prevSocket, state->self);
}

/**
 * @brief Show's state variables
 * 
 * @param state 
 */
void show(nodeState *state){
    printf("Predecessor:\n\tKey:%d \n\t IP:%s \n\tPort:%d", state->prev->key, state->prev->ip, state->prev->port);
    printf("Successor:\n\tKey:%d \n\t IP:%s \n\tPort:%d", state->next->key, state->next->ip, state->next->port);
    printf("Self:\n\tKey:%d \n\t IP:%s \n\tPort:%d", state->self->key, state->self->ip, state->self->port);
    if(state->SC->fd != -1)
        printf("Shortcut:\n\tKey:%d \n\t IP:%s \n\tPort:%d", state->SC->key, state->SC->ip, state->SC->port);
}

void find(nodeState *state, char *info, message *msg, int keys[32], bool isSystemCall){
    char* trash;
    int k;

    if(info != NULL) sscanf(info, "%s %d", trash, k);
    if(msg != NULL) k=msg->nodeKey;

    if(dist(state->self->key,k) > dist(state->next->key, k)){
        //send FND message to next
    }else{
        if(isSystemCall){
            printf("Key %d found in node with:\n\tKey:%d \n\tIP:%s \n\tPort:%d", k, state->self->key, state->self->ip, state->self->port);
        }else{
            //send RSP message to next or SC
            if(state->SC->fd != -1){
                if(dist(state->SC->key, k) < dist(state->next->key, k)){
                    //Send RSP to SC and begin (still need to create) timeout protocol until i receive an ACK and send to next
                }else{
                    msgRSP()

                }
            }
        }
    }


}

/**
 * @brief Messages PRED message
 * 
 * @param fd fd to write do
 * @param nextPred My next, next node's pred info
 */
void msgPred(int fd, nodeInfo *nextPred){
    message msg;
    strcpy(msg.command, "PRED");
    strcpy(msg.ip, nextPred->ip);
    msg.port=nextPred->port;
    msg.nodeKey=nextPred->port;
    talkTCP(fd, &msg);
}

/**
 * @brief Messages SELF message
 * 
 * @param fd fd to write do
 * @param self My variables
 */
void msgSelf(int fd, nodeInfo *self){
    message msg;
    strcpy(msg.command, "SELF");
    strcpy(msg.ip,self->ip);
    msg.nodeKey=self->key;
    msg.port=self->port;
    talkTCP(fd, &msg);
}

/**
 * @brief 
 * 
 * @param fd 
 * @param node Information of the node 
 * @param isTCP Is it done trough TCP or UDP protocol
 */
void msgRSP(int fd, nodeInfo *node, bool isTCP){
    message msg;
    strcpy(msg.command,"RSP");
    strcpy(msg.ip, node->ip);
    msg.port=node->port;
    msg.nodeKey=node->key;
    if(isTCP) talkTCP(fd, &msg);
    else //talkUDP()
}
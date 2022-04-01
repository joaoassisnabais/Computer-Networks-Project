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

#define dist(start,end) ((end-start)%32)


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
        initState(0, state, NULL, &next, -1, state->next->fd);
        if(dist(state->self->key,state->old->key) > dist(state->self->key, state->next->key)) pred(state->old->fd, state->next);
    }

    if(strcmp(msg->command, "PRED") == 0){
        nodeInfo prev;

        closeTCP(state->prev->fd);
        FD_CLR(state->prev->fd, current);
        free(state->prev);

        prev.key=msg->nodeKey;
        prev.port=msg->port;
        strcpy(prev.ip, msg->ip);

        prev.fd = clientTCP(&msg->ip, msg->port);
        initState(0, &state, &prev, NULL, prev.fd, -1);
        FD_SET(prev.fd, current);
        msgSelf(prev.fd, state->self);
    }

    if(strcmp(msg->command, ""))
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
 * @brief Messages PRED message
 * 
 * @param fd fd to write do
 * @param nextPred My next, next node's pred info
 */
void msgpred(int fd, nodeInfo *nextPred){
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
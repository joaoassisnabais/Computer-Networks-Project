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
 */
void rcv_msg(message *msg, nodeState *state){

    if(strcmp(msg->command, "SELF") == 0){
        nodeInfo next;
        next.key=msg->nodeKey;
        next.port=msg->port;
        strcpy(next.ip, msg->ip); 
        initState(0, state, NULL, &next, -1, state->next->fd);
        if(dist(state->self->key,state->old->key) > dist(state->self->key, state->next->key)) pred(state->old->fd, state->next);
    }

    if(strcmp(msg->command, "PRED") == 0){
        closeTCP(state->prev->)


    }
}


void msgSelf(message *msg, nodeInfo *self){
    strcpy(msg->command, "SELF");
    strcpy(msg->ip,self->ip);
    msg->nodeKey=self->key;
    msg->port=self->port;
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

    sscanf(info, "%s %d %s %d", trash, prev.key, prev.ip, prev.port);
    prevSocket = clientTCP( &state->prev->ip, state->prev->port);
    
    initState(0, &state, &prev, NULL, prevSocket, -1);
}

void pred(int fd, nodeInfo *nextPred){
    message msg;
    strcpy(msg.command, "PRED");
    strcpy(msg.ip, nextPred->ip);
    msg.port=nextPred->port;
    msg.nodeKey=nextPred->port;
    talkTCP(fd, &msg);
}
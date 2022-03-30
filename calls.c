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
int pentry(nodeState *state, char *info){
    char* trash;
    nodeInfo prev, next;
    int prevSocket;

    sscanf(info, "%s %d %s %d", trash, prev.key, prev.ip, prev.port);
    initState(0, &state, &prev, NULL);
    prevSocket = clientTCP(&state->prev->ip, state->prev->port);
    
    return prevSocket;
}
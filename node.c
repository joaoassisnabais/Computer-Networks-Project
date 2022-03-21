#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "node.h"

nodeState create_node(nodeInfo prevNode){   //create new nodeState struct with previous node knowledge
    
    nodeState newNode;
    newNode.prev = malloc(sizeof(nodeInfo)); 

    strcpy(newNode.prev->ip, prevNode.ip);
    newNode.prev->port = prevNode.port;
    newNode.prev->key = prevNode.key;

    return newNode;
}

void change_prev(nodeState *currNode, nodeInfo newNode){
    strcmp(currNode->next->ip, newNode.ip);
    currNode->next->key = newNode.key;
    currNode->next->port = newNode.port;
}
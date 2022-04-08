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
        printf("\nReceived Self from node %d with Port: %d and IP: %s", msg->nodeKey, msg->port, msg->ip);
        nodeInfo next;
        next.key=msg->nodeKey;
        next.port=msg->port;
        strcpy(next.ip, msg->ip);
        
        //checks if it's the node is alone in the ring
        if(state->next->key==state->self->key){
            strcpy(msg->command,"PRED");
        }

        initState(0, state, NULL, &next, -1, state->next->fd); //redundancy in fd, there would be no need tp send fd but to have a complete function sent it anyway
        
        //if an old socket exists check what's closer to see if it's a node leaving or a new node entering
        if(state->old->fd != -1){
            if(dist(state->old->key,state->self->key) > dist(state->next->key, state->self->key)){ 
                msgPred(state->old->fd, state->next);
            }
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

        prev.fd = clientTCP(msg->ip, msg->port);
        initState(0, state, &prev, NULL, prev.fd, -1);
        FD_SET(prev.fd, current);
        msgSelf(prev.fd, state->self);
    }

    if(strcmp(msg->command, "FND") == 0){
        find(state, NULL, msg);
    }

    if(strcmp(msg->command, "RSP") == 0){

        //See if the RSP is for me        
        if(msg->searchKey == state->self->key){   //It's for me
            if(seq[msg->sequenceN] != -1){
                printf("\nFound Key:%d in node with: \n\tKey:%d \n\tIP:%s \n\tPort:%d",seq[msg->sequenceN], msg->nodeKey, msg->ip, msg->port);
                seq[msg->sequenceN] = -1;
            }
        }else{  //It's not for me
            //Checks if SC is closer
            if(dist(state->SC->key, msg->searchKey) < dist(state->next->key, msg->searchKey)){  //SC is closer
                msgRSP(state->SC->fd, NULL, msg, 0, -1);
            }
            msgRSP(state->next->fd, NULL, msg, 1, -1);
        }

    }
}

/**
 * @brief Entry in the ring knowing the predecessor
 * 
 * @param state State Variables
 * @param info String with the predecessors variables
 */
void pentry(nodeState *state, char *info){
    nodeInfo prev;

    if(info != NULL){
        sscanf(info, "%*s %d %s %d", &prev.key, prev.ip, &prev.port);
    }
    prev.fd = clientTCP(prev.ip, prev.port);
    
    initState(0, state, &prev, NULL, prev.fd, -1);
    msgSelf(prev.fd, state->self);
}

/**
 * @brief Show's state variables
 * 
 * @param state 
 */
void show(nodeState *state){
    printf("\nPredecessor:\n\tKey:%d \n\tIP:%s \n\tPort:%d", state->prev->key, state->prev->ip, state->prev->port);
    printf("\nSelf:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n", state->self->key, state->self->ip, state->self->port);
    printf("\nSuccessor:\n\tKey:%d \n\tIP:%s \n\tPort:%d", state->next->key, state->next->ip, state->next->port);
    if(state->SC->fd != -1)
        printf("Shortcut:\n\tKey:%d \n\t IP:%s \n\tPort:%d", state->SC->key, state->SC->ip, state->SC->port);
}

/**
 * @brief checks if self has the key and responds apropriately
 * 
 * @param state State Variables
 * @param info if function is called by user it's the buffer from stdin
 * @param msg if FND msg is received this is that message
 * @param seq stores sequence numbers to keep track of requests
 * @param findI sequence number of this call -> find function index
 */
void find(nodeState *state, char *info, message *msg){
    int k;
    bool isSystemCall;

    if(info != NULL){   //system call
        isSystemCall=true;
        sscanf(info, "%*s %d", &k);
    } 
    else{   //not a system call
        isSystemCall=false;
        k=msg->searchKey;     
    }

    //Checks if k key is in self
    if(dist(state->self->key,k) > dist(state->next->key, k)){   //key isn't in self

        if(isSystemCall){
            if(seq[findI] != -1) perror("Sequence number already in use");  //can't do a repeated find
            seq[findI]=k;
        }

        //Checks if SC exists
        if(state->SC->fd != -1){
            //checks if SC is closer than next
            if(dist(state->SC->key, k) < dist(state->next->key, k)){    //SC is closer than next
                msgFND(state->SC->fd, state->self, msg, 1, k);                
            }
            msgFND(state->next->fd, state->self, msg, 0, k);        //always send message to next in case UDP message is slower                
        }
    }
    //key is in self
    else{
        if(isSystemCall){
            printf("Key %d found in node with:\n\tKey:%d \n\tIP:%s \n\tPort:%d", k, state->self->key, state->self->ip, state->self->port);
        }else{
            //check if SC is closer than next
            if(state->SC->fd != -1){
                if(dist(state->SC->key, k) < dist(state->next->key, k)){
                    msgRSP(state->SC->fd, state->self, msg, 1, msg->nodeKey);
                }
                msgRSP(state->next->fd, state->self, msg, 1, msg->nodeKey); //always send message to next in case UDP message is slower
            }
        }
    }
}


/**
 * @brief Sends PRED message
 * 
 * @param fd File descriptor to write to
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
 * @brief Sends SELF message
 * 
 * @param fd File descriptor to write to
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
 * @brief Sends RSP message
 * 
 * @param fd File descriptor to write to
 * @param node Node information to fill in RSP
 * @param msg In case I just need to resend the message 
 * @param isTCP If it's sent to the shortcut or to the next node
 * @param k key from the node that asked for it
 */
void msgRSP(int fd, nodeInfo *node, message *msg, bool isTCP, int k){
   if(msg == NULL) {    //system call
        message aux;
        msg = &aux;     //create msg struct with system call inputs

        strcpy(msg->command,"RSP");
        strcpy(msg->ip, node->ip);
        msg->port=node->port;
        msg->nodeKey=node->key;
        msg->searchKey=k;
        //msg.sequenceN = ?;
    }
    if(isTCP) talkTCP(fd, msg);
    //else talkUDP()
}

/**
 * @brief Sends FND message
 * 
 * @param fd File descriptor to write to
 * @param node Node information to fill in RSP
 * @param msg In case I just need to resend the message 
 * @param isTCP If it's sent to the shortcut or to the next node
 * @param k key to be found
 */
void msgFND(int fd, nodeInfo *node, message *msg, bool isTCP, int k){
    if(msg == NULL) {    //system call
        message aux;
        msg = &aux;     //create msg struct with system call inputs

        strcpy(msg->command,"FND");
        strcpy(msg->ip, node->ip);
        msg->port=node->port;
        msg->nodeKey=node->key;
        msg->searchKey=k;
        //msg.sequenceN = ?;
    }

    if(isTCP) talkTCP(fd, msg);
    //else talkUDP()
}
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

#define max(A,B) ((A)>=(B)?(A):(B))

/**
 * @brief Receives a message and decides what to do with it (message control)
 * 
 * @param msg  The message itself
 * @param state State Variables
 * @param current Current fd_set so it can be updated
 * @param maxfd current maximum file descriptor
 */
void rcv_msg(message *msg, nodeState *state, fd_set *current, int *maxfd){

    /**
     * @brief Receives and deals with SELF message
     * 
     */
    if(strcmp(msg->command, "SELF") == 0){
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
            if(dist(state->self->key,state->old->key) > dist(state->self->key, state->next->key)){ 
                msgPred(state->old->fd, state->next);
            }
        }
    }

    /**
     * @brief Receives and deals with PRED message
     * 
     */
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
        *maxfd=max(*maxfd, prev.fd);
        msgSelf(prev.fd, state->self);
    }

    /**
     * @brief Receives and deals with FND message
     * 
     */
    if(strcmp(msg->command, "FND") == 0){
        find(state, NULL, msg);
    }

    /**
     * @brief Receives and deals with RSP message
     * 
     */
    if(strcmp(msg->command, "RSP") == 0){

        //See if the RSP is for me        
        if(msg->searchKey == state->self->key){   //It's for me
            if(seq[msg->sequenceN] != -1){
                printf("Key %d found in node with:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n",seq[msg->sequenceN], msg->nodeKey, msg->ip, msg->port);
                seq[msg->sequenceN] = -1;
            }
        }else{  //It's not for me
            
            //Checks if SC exists
            if(state->SC->fd!=-1){
                
                //Checks if SC is closer
                if(dist(state->SC->key, msg->searchKey) < dist(state->next->key, msg->searchKey)){  //SC is closer
                    msgRSP(state->SC, NULL, msg, 0, -1, -1);
                }

                //SC exists but it's not the closest
                else{ 
                    msgRSP(state->next, NULL, msg, 1, -1, -1);
                }
            }
            
            //SC doesn't exist
            else{
                msgRSP(state->next, NULL, msg, 1, -1, -1);
            }
        }   
    }

    /**
     * @brief Receives and deals with EFND message
     * 
     */
    if(strcmp(msg->command, "EFND") == 0){
        
    }

    /**
     * @brief Receives and deals with EPRED message
     * 
     */
    if(strcmp(msg->command, "EPRED") == 0){

    }

    /**
     * @brief Receives and deals with ACK message
     * 
     */
    if(strcmp(msg->command, "ACK") == 0){

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
    if(state->prev->fd != -1 || state->prev->key==state->self->key)
        printf("\nPredecessor:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n", state->prev->key, state->prev->ip, state->prev->port);
    
    printf("Self:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n", state->self->key, state->self->ip, state->self->port);
    
    if(state->next->fd != -1 || state->next->key==state->self->key)
        printf("Successor:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n", state->next->key, state->next->ip, state->next->port);
    
    if(state->SC->fd != -1)
        printf("Shortcut:\n\tKey:%d \n\t IP:%s \n\tPort:%d \n", state->SC->key, state->SC->ip, state->SC->port);
}

/**
 * @brief Leaves the current ring
 * 
 * @param state State Variables
 * @param current Current fd set
 * @param maxfd Maximum file descriptor to send to select
 */
void leave(nodeState *state, fd_set *current, int *maxfd, int TCPsocket, int UDPsocket){
    if(state->prev->fd != -1){       //if its new there's no tcp connection to close
        closeTCP(state->prev->fd);
    }
    if(state->next->fd != -1){
        msgPred(state->next->fd, state->prev);
        closeTCP(state->next->fd);
    }
    closeTCP(TCPsocket);
    closeTCP(UDPsocket);
    FD_ZERO(current);
    FD_SET(0, current);
    maxfd=0;
    state->old->fd=-1;
    state->SC->fd=-1;
    state->next->fd=-1;
    state->prev->fd=-1;
    state->next->key=-1;
    state->prev->key=-1;
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
            if(findI==99) findI=-1;
            findI+=1;
            if(seq[findI] != -1) perror("Sequence number already in use");  //can't do a repeated find
            seq[findI]=k;
        }

        //Checks if SC exists
        if(state->SC->fd != -1){
            //checks if SC is closer than next
            if(dist(state->SC->key, k) < dist(state->next->key, k)){    //SC is closer than next
                msgFND(state->SC, state->self, msg, 0, k);                
            }
            //SC exists but it's not the closest
            else{
                msgFND(state->next, state->self, msg, 1, k);
            }                
        }
        //SC doesn't exist
        else{
            msgFND(state->next, state->self, msg, 1, k);
        }
    }
    //key is in self
    else{
        if(isSystemCall){
            printf("Key %d found in node with:\n\tKey:%d \n\tIP:%s \n\tPort:%d \n", k, state->self->key, state->self->ip, state->self->port);
        }else{
            //check if SC is closer than next
            if(state->SC->fd != -1){
                if(dist(state->SC->key, k) < dist(state->next->key, k)){
                    msgRSP(state->SC, state->self, NULL, 0, msg->nodeKey,msg->sequenceN);
                }
                //SC exists but it's not the closest
                else{
                    msgRSP(state->next, state->self, NULL, 1, msg->nodeKey, msg->sequenceN); 
                }
            }
            //SC doesn't exist
            else{
                msgRSP(state->next, state->self, NULL, 1, msg->nodeKey, msg->sequenceN);
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
    msg.nodeKey=nextPred->key;
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
 * @param receiver Node information of receiver node
 * @param node Node information to fill in RSP
 * @param msg In case I just need to resend the message 
 * @param isTCP If it's sent to the shortcut or to the next node
 * @param k key from the node that asked for it
 * @param seqn Sequence number from FND
 */
void msgRSP(nodeInfo *receiver, nodeInfo *node, message *msg, bool isTCP, int k, int seqn){
    if(msg == NULL) {    //system call
        message aux;
        msg = &aux;     //create msg struct with system call inputs

        strcpy(msg->command,"RSP");
        strcpy(msg->ip, node->ip);
        msg->port=node->port;
        msg->nodeKey=node->key;
        msg->searchKey=k;
        msg->sequenceN = seqn;
    }
    if(isTCP) talkTCP(receiver->fd, msg);
    else clientTalkUDP(receiver->ip, receiver->port, msg);
}

/**
 * @brief Sends FND message
 * 
 * @param receiver Node information of receiver node
 * @param node Node information to fill in RSP
 * @param msg In case I just need to resend the message 
 * @param isTCP If it's sent to the shortcut or to the next node
 * @param k key to be found
 */
void msgFND(nodeInfo *receiver, nodeInfo *node, message *msg, bool isTCP, int k){
    if(msg == NULL) {    //system call
        message aux;
        msg = &aux;     //create msg struct with system call inputs

        strcpy(msg->command,"FND");
        strcpy(msg->ip, node->ip);
        msg->port=node->port;
        msg->nodeKey=node->key;
        msg->searchKey=k;
        msg->sequenceN = findI;
    }

    if(isTCP) talkTCP(receiver->fd, msg);
    else clientTalkUDP(receiver->ip, receiver->port, msg);
}
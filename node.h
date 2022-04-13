#include <stdbool.h>
#ifndef node_h
#define node_h

typedef struct node_info {
    int key;
    char ip[16];
    int port;
    int fd;
} nodeInfo;

typedef struct node_state {
    nodeInfo *next;
    nodeInfo *self;
    nodeInfo *prev;
    nodeInfo *old;
    nodeInfo *SC;
} nodeState;

typedef struct comms_message{
    char command[6];
    int nodeKey;    //key from sending node (i)
    int sequenceN;
    int searchKey;  //key to search (k)
    char ip[16];
    int port;
} message;

extern int seq[100], findI;
extern struct sockaddr addrNewNode, addrResendUDP, addrACKSendUDP;
extern socklen_t addrlenNewNode, addrlenResendUDP, addrlenACKSendUDP;
extern bool flagACK;
extern message msgResendUDP;

void core(int selfKey, char *IP, int selfPort);
void initState(bool isNew, nodeState *state, nodeInfo *prev, nodeInfo *next, int pfd, int nfd);
int dist(int start, int end);

#endif
#include <stdbool.h>
#define node_h
//#ifndef node_h

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
    int nodeKey;    //key from sending node
    int sequenceN;
    int searchKey;  //key to search
    char ip[16];
    int port;
} message;

void core(int selfKey, char *IP, int selfPort);
void initState(bool isNew, nodeState *state, nodeInfo *prev, nodeInfo *next, int pfd, int nfd);

//#endif
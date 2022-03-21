#define node_h
//#ifndef node_h

typedef struct node_info {
    int key;
    char ip[16];
    int port;
} nodeInfo;

typedef struct node_state {
    nodeInfo *next;
    nodeInfo *self;
    nodeInfo *prev;
} nodeState;

//#endif
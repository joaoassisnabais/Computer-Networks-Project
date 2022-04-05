#include <stdbool.h>
#include "node.h"
#define calls_h
//#ifndef node_h

void pentry(nodeState *state, char *info);
void rcv_msg(message *msg, nodeState *state, fd_set *current);
void msgPred(int fd, nodeInfo *nextPred);
void msgSelf(int fd, nodeInfo *self);
void show(nodeState *state);

//#endif
#include <stdbool.h>
#include "node.h"
#ifndef calls_h
#define calls_h

void pentry(nodeState *state, char *info);
void rcv_msg(message *msg, nodeState *state, fd_set *current, int *maxfd);
void msgPred(int fd, nodeInfo *nextPred);
void msgSelf(int fd, nodeInfo *self);
void msgRSP(nodeInfo *receiver, nodeInfo *node, message *msg, bool isTCP, int k);
void msgFND(nodeInfo *receiver, nodeInfo *node, message *msg, bool isTCP, int k);
void show(nodeState *state);
void find(nodeState *state, char *info, message *msg);

#endif
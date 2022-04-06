#include <stdbool.h>
#include "node.h"
#ifndef calls_h
#define calls_h

void pentry(nodeState *state, char *info);
void rcv_msg(message *msg, nodeState *state, fd_set *current);
void msgPred(int fd, nodeInfo *nextPred);
void msgSelf(int fd, nodeInfo *self);
void msgRSP(int fd, nodeInfo *node, message *msg, bool isTCP, int k);
void msgFND(int fd, nodeInfo *node, message *msg, bool isTCP, int k);
void show(nodeState *state);
void find(nodeState *state, char *info, message *msg);

#endif
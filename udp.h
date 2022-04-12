#ifndef udp_h
#define udp_h
#include "node.h"

int serverUDP(char *IP, int port);
int clientTalkUDP(char *serverIP, int serverPort, message *msg);

#endif
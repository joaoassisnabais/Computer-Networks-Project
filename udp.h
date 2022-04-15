#ifndef udp_h
#define udp_h
#include "node.h"

int serverUDP(char *IP, int port);
void clientTalkUDP(char *serverIP, int serverPort, message *msg);
void receive_messageUDP(message *msg);

#endif
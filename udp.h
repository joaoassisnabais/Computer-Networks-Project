#ifndef udp_h
#define udp_h
#include "node.h"

int serverUDP(char *IP, int port);
void clientTalkUDP(char *serverIP, int serverPort, message *msg, int socketTCP);
void receive_messageUDP(message *msg);

#endif
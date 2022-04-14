#ifndef udp_h
#define udp_h
#include "node.h"

extern struct sockaddr addr;
extern socklen_t addrlen;

int serverUDP(char *IP, int port);
int clientTalkUDP(char *serverIP, int serverPort, message *msg, int fd);
void receive_messageUDP(int serverSocket, message *msg);

#endif
#ifndef udp_h
#define udp_h
#include "node.h"

extern struct sockaddr addr;
extern socklen_t addrlen;
extern int serverSocketUDP, serverSocketTCP;

int serverUDP(char *IP, int port);
int clientTalkUDP(char *serverIP, int serverPort, message *msg);
void receive_messageUDP(int serverSocket, message *msg);

#endif
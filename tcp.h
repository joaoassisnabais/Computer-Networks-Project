#define tcp_h
#include "node.h"
//#ifndef tcp_h

int serverTCP(int port);
int accept_connectionTCP(int fd);
int clientTCP(char *serverIP, int serverPort);
int readTCP(int fd, message *msg);
void closeTCP(int fd);
void talkTCP(int fd, message *msg);

//#endif
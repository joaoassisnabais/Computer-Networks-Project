#define tcp_h
//#ifndef tcp_h

int serverTCP(int port);
int accept_connectionTCP(int fd);
int clientTCP(char *serverIP, int serverPort);

//#endif
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "node.h"

//NEED TO PROTECT AGAINST SIGPIPE


int main(int argc, char **argv){

    int i, port;
    char ip[16];

    sscanf(argv[1], "%d", &i);
    strcpy(ip,argv[2]);
    sscanf(argv[3], "%d", &port);

    core(i, &ip, port);
    
    return 0;
}
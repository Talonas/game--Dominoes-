#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket.h"

char *getMessage(int sock, int length) {
    char *readbuff = (char *)malloc(512);

    if (length == 0) {
        int recievedBytes = recv(sock, readbuff, 512, 0);
        readbuff[recievedBytes+1] = '\0';
    }
    if (length > 0) {
        recv(sock, readbuff, length, 0);
        readbuff[length+1] = '\0';
    }

    return readbuff;
}

void sendMessage(int sock, int length, char *msg) {
    int bytes_sent = 0;
    int bytes_left = length;
    int bytes;

    while (bytes_sent < length) {
        bytes = send(sock, msg + bytes_sent, bytes_left, 0);
        if (bytes == -1)
            break;
        bytes_sent += bytes;
        bytes_left -= bytes;
    }
}

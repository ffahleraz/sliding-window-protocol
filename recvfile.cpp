#include<iostream>

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 256

using namespace std;

int main(int argc, char * argv[]) {
    int socket_fd;
    int port;
    ssize_t buff_size;
    struct sockaddr_in server_addr, client_addr;

    if (argc == 3) {
        buff_size = atoi(argv[1]);
        port = atoi(argv[2]);
    } else {
        cerr << "usage: ./recvfile <buffer_size> <port>" << endl;
        return 1; 
    }

    memset(&server_addr, 0, sizeof(server_addr)); 
    memset(&client_addr, 0, sizeof(client_addr)); 
      
    /* Fill server address data structure */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(port);

    /* Create socket file descriptor */ 
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed" << endl;
        return 1;
    }

    /* Bind socket to server address */
    if (bind(socket_fd, (const struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0) { 
        cerr << "socket binding failed" << endl;
        return 1;
    }

    char buffer[buff_size];
    ssize_t recv_size;
    socklen_t ca_len;
    recv_size = recvfrom(socket_fd, (char * )buffer, buff_size, 
            MSG_WAITALL, (struct sockaddr * ) &client_addr, &ca_len);
    buffer[recv_size] = '\0';

    cout << buffer << endl;
}
#include<iostream>

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

int main(int argc, char * argv[]) { 
    int socket_fd;
    int buff_size;
    char *dest_ip;
    int dest_port;
    struct hostent *dest_hnet;
    struct sockaddr_in dest_addr;

    if (argc == 4) {
        buff_size = atoi(argv[1]);
        dest_ip = argv[2];
        dest_port = atoi(argv[3]);
    } else {
        cerr << "usage: ./sendfile <buffer_size> <destination_ip> <destination_port>" << endl;
        return 1; 
    }

    /* Get hostnet from destination hostname or IP address */ 
    dest_hnet = gethostbyname(dest_ip); 
    if (!dest_hnet) {
        cerr << "unknown host: " << dest_ip << endl;
        return 1;
    }

    /* Fill destination address data structure */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    bcopy(dest_hnet->h_addr, (char * )&dest_addr.sin_addr, 
            dest_hnet->h_length); 
    dest_addr.sin_port = htons(dest_port);

    /* Create socket file descriptor */ 
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed" << endl;
        return 1;
    }

    char buffer[buff_size];
    strcpy(buffer, "hello");

    sendto(socket_fd, buffer, buff_size, MSG_CONFIRM, 
            (const struct sockaddr *) &dest_addr, sizeof(dest_addr)); 
    cout << buffer << endl;
}
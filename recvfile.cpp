#include<iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#include "helpers.h"

using namespace std;

int main(int argc, char * argv[]) {
    int socket_fd;
    unsigned int port;
    unsigned int window_size;
    ssize_t buff_size;
    struct sockaddr_in server_addr, client_addr;

    if (argc == 4) {
        window_size = atoi(argv[1]);
        buff_size = atoi(argv[2]);
        port = atoi(argv[3]);
    } else {
        cerr << "usage: ./recvfile <window_size> <buffer_size> <port>" << endl;
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

    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    char ack[ACK_SIZE];
    size_t frame_size;
    size_t data_size;
    socklen_t client_addr_size;
    bool eot;
    bool frame_error;
    unsigned int recv_seq_num;
    
    ssize_t lfr, laf;
    bool window_recv_mask[window_size];
    for (int i = 0; i < window_size; i++) {
        window_recv_mask[i] = false;
    }
    
    lfr = -1;
    laf = lfr + window_size;
    while (true) {
        frame_size = recvfrom(socket_fd, (char *)frame, MAX_FRAME_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &client_addr, 
                &client_addr_size);
        frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);

        if (recv_seq_num <= laf) {
            create_ack(recv_seq_num, ack, frame_error);
            sendto(socket_fd, ack, ACK_SIZE, MSG_CONFIRM, 
                    (const struct sockaddr *) &client_addr, client_addr_size);
            
            if (!frame_error) {
                if (recv_seq_num == lfr + 1) {
                    unsigned int shift = 1;
                    for (unsigned int i = 1; i < window_size; i++) {
                        if (!window_recv_mask[i]) break;
                        shift += 1;
                    }
                    for (unsigned int i = 0; i < window_size - shift; i++) {
                        window_recv_mask[i] = window_recv_mask[i + shift];
                    }
                    for (unsigned int i = window_size - shift; i < window_size; i++) {
                        window_recv_mask[i] = false;
                    }
                    lfr += shift;
                    laf = lfr + window_size;
                } else if (recv_seq_num > lfr + 1) {
                    window_recv_mask[recv_seq_num - (lfr + 1)] = true;
                }
                data[data_size] = '\0';
                cout << "[RECV FRAME " << recv_seq_num << "] " << data_size << " bytes" << endl;
                cout << "[SENT ACK " << recv_seq_num << "]" << endl;
            } else {
                cout << "[ERR FRAME " << recv_seq_num << "] " << data_size << " bytes" << endl;
                cout << "[SENT NAK " << recv_seq_num << "]" << endl;
            }
        } else {
            cout << "[IGN FRAME " << recv_seq_num << "] " << data_size << " bytes" << endl;
        }
    }
}
#include <iostream>
#include <thread>
#include <ctime>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#include "helpers.h"

using namespace std;

int socket_fd;
unsigned int window_size;
char *dest_ip;
int dest_port;
ssize_t buff_size;
struct hostent *dest_hnet;
struct sockaddr_in server_addr, client_addr;
bool *window_recv_mask;

void listen_ack() {
    while (true) {
        char ack[ACK_SIZE];
        size_t ack_size;
        unsigned int ack_seq_num;
        bool ack_error;
        bool ack_conf;

        socklen_t server_addr_size;
        ack_size = recvfrom(socket_fd, (char *)ack, ACK_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &server_addr, 
                &server_addr_size);
        ack_error = read_ack(&ack_seq_num, &ack_conf, ack);

        cout << "[RECV ACK " << ack_seq_num << "]" << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc == 5) {
        window_size = atoi(argv[1]);
        buff_size = atoi(argv[2]);
        dest_ip = argv[3];
        dest_port = atoi(argv[4]);
    } else {
        cerr << "usage: ./sendfile <window_size> <buffer_size> <destination_ip> <destination_port>" << endl;
        return 1; 
    }

    /* Get hostnet from server hostname or IP address */ 
    dest_hnet = gethostbyname(dest_ip); 
    if (!dest_hnet) {
        cerr << "unknown host: " << dest_ip << endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); 
    memset(&client_addr, 0, sizeof(client_addr)); 

    /* Fill server address data structure */
    server_addr.sin_family = AF_INET;
    bcopy(dest_hnet->h_addr, (char *)&server_addr.sin_addr, 
            dest_hnet->h_length); 
    server_addr.sin_port = htons(dest_port);

    /* Fill client address data structure */
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY; 
    client_addr.sin_port = htons(0);

    /* Create socket file descriptor */ 
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed" << endl;
        return 1;
    }

    /* Bind socket to client address */
    if (bind(socket_fd, (const struct sockaddr *)&client_addr, 
            sizeof(client_addr)) < 0) { 
        cerr << "socket binding failed" << endl;
        return 1;
    }

    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    size_t frame_size;
    size_t data_size;
    
    /* Initialize sliding window variables */
    unsigned int seq_num = 0;
    unsigned int lar, lfs;
    time_t window_sent_time[window_size];
    window_recv_mask = new bool[window_size];
    
    for (int i = 0; i < window_size; i++) {
        window_recv_mask[i] = false;
    }

    /* Start thread to listen for ack */
    thread recv_thread(listen_ack);

    while (seq_num < 24) {
        strcpy(data, "hello asu lu semua lkonto");
        data_size = (size_t)strlen(data);
        frame_size = create_frame(seq_num, frame, data, data_size);

        sendto(socket_fd, frame, frame_size, MSG_CONFIRM, 
                (const struct sockaddr *) &server_addr, sizeof(server_addr));

        cout << "[SENT FRAME " << seq_num << "] " << data << endl;
        seq_num ++;
    }

    recv_thread.join();
    delete [] window_recv_mask;
}
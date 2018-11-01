#include <iostream>
#include <thread>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#include "helpers.h"

using namespace std;

int socket_fd;
struct sockaddr_in server_addr, client_addr;

void send_ack() {
    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    char ack[ACK_SIZE];
    int frame_size;
    int data_size;
    socklen_t client_addr_size;
    
    int recv_seq_num;
    bool frame_error;
    bool eot;

    /* Listen for frames and send ack */
    while (true) {
        frame_size = recvfrom(socket_fd, (char *)frame, MAX_FRAME_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &client_addr, 
                &client_addr_size);
        frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);

        create_ack(recv_seq_num, ack, frame_error);
        sendto(socket_fd, ack, ACK_SIZE, 0, 
                (const struct sockaddr *) &client_addr, client_addr_size);
    }
}

int main(int argc, char * argv[]) {
    int port;
    int window_len;
    int max_buffer_size;
    char *fname;

    if (argc == 5) {
        fname = argv[1];
        window_len = (int) atoi(argv[2]);
        max_buffer_size = MAX_DATA_SIZE * (int) atoi(argv[3]);
        port = atoi(argv[4]);
    } else {
        cerr << "usage: ./recvfile <filename> <window_size> <buffer_size> <port>" << endl;
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
    if (::bind(socket_fd, (const struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0) { 
        cerr << "socket binding failed" << endl;
        return 1;
    }

    FILE *file = fopen(fname, "wb");
    char buffer[max_buffer_size];
    int buffer_size;

    /* Initialize sliding window variables */
    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    char ack[ACK_SIZE];
    int frame_size;
    int data_size;
    int lfr, laf;
    int recv_seq_num;
    bool eot;
    bool frame_error;

    /* Receive frames until EOT */
    bool recv_done = false;
    int buffer_num = 0;
    while (!recv_done) {
        buffer_size = max_buffer_size;
        memset(buffer, 0, buffer_size);
    
        int recv_seq_count = (int) max_buffer_size / MAX_DATA_SIZE;
        bool window_recv_mask[window_len];
        for (int i = 0; i < window_len; i++) {
            window_recv_mask[i] = false;
        }
        lfr = -1;
        laf = lfr + window_len;
        
        /* Receive current buffer with sliding window */
        while (true) {
            socklen_t client_addr_size;
            frame_size = recvfrom(socket_fd, (char *) frame, MAX_FRAME_SIZE, 
                    MSG_WAITALL, (struct sockaddr *) &client_addr, 
                    &client_addr_size);
            frame_error = read_frame(&recv_seq_num, data, &data_size, &eot, frame);

            create_ack(recv_seq_num, ack, frame_error);
            sendto(socket_fd, ack, ACK_SIZE, 0, 
                    (const struct sockaddr *) &client_addr, client_addr_size);

            if (recv_seq_num <= laf) {
                if (!frame_error) {
                    int buffer_shift = recv_seq_num * MAX_DATA_SIZE;

                    if (recv_seq_num == lfr + 1) {
                        memcpy(buffer + buffer_shift, data, data_size);

                        int shift = 1;
                        for (int i = 1; i < window_len; i++) {
                            if (!window_recv_mask[i]) break;
                            shift += 1;
                        }
                        for (int i = 0; i < window_len - shift; i++) {
                            window_recv_mask[i] = window_recv_mask[i + shift];
                        }
                        for (int i = window_len - shift; i < window_len; i++) {
                            window_recv_mask[i] = false;
                        }
                        lfr += shift;
                        laf = lfr + window_len;
                    } else if (recv_seq_num > lfr + 1) {
                        if (!window_recv_mask[recv_seq_num - (lfr + 1)]) {
                            memcpy(buffer + buffer_shift, data, data_size);
                            window_recv_mask[recv_seq_num - (lfr + 1)] = true;
                        }
                    }

                    /* Set max sequence to sequence of frame with EOT */ 
                    if (eot) {
                        buffer_size = buffer_shift + data_size;
                        recv_seq_count = recv_seq_num + 1;
                        recv_done = true;
                    }
                }
            }
            
            /* Move to next buffer if all frames in current buffer has been received */
            if (lfr >= recv_seq_count - 1) break;
        }

        cout << "\r" << "[RECEIVED " << (unsigned long long) buffer_num * (unsigned long long) 
                max_buffer_size + (unsigned long long) buffer_size << " BYTES]" << flush;
        fwrite(buffer, 1, buffer_size, file);
        buffer_num += 1;
    }

    fclose(file);

    /* Start thread to keep sending requested ack to sender for 3 seconds */
    cout << endl << "[STANDBY TO SEND ACK FOR 3 SECONDS...]" << endl;
    thread stdby_thread(send_ack);
    sleep_for(3000);
    stdby_thread.detach();

    return 0;
}
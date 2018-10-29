#include <iostream>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#include "helpers.h"

#define TIMEOUT 100

#define current_time chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define elapsed_time(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()

using namespace std;

int socket_fd;
char *dest_ip;
int dest_port;
ssize_t buff_size;
struct hostent *dest_hnet;
struct sockaddr_in server_addr, client_addr;

unsigned int window_size;
bool *window_ack_mask;
time_stamp *window_sent_time;
unsigned int lar, lfs;

time_stamp TMIN = current_time();

void listen_ack() {
    char ack[ACK_SIZE];
    size_t ack_size;
    unsigned int ack_seq_num;
    bool ack_error;
    bool ack_conf;

    while (true) {
        socklen_t server_addr_size;
        ack_size = recvfrom(socket_fd, (char *)ack, ACK_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &server_addr, 
                &server_addr_size);
        ack_error = read_ack(&ack_seq_num, &ack_conf, ack);

        if (!ack_error) {
            if (ack_seq_num >= lar + 1 && ack_seq_num <= lfs) {
                if (ack_conf) {
                    window_ack_mask[ack_seq_num - (lar + 1)] = true;
                    cout << "[RECV ACK " << ack_seq_num << "]" << endl;
                } else {
                    window_sent_time[ack_seq_num - (lar + 1)] = TMIN;
                    cout << "[RECV NAK " << ack_seq_num << "]" << endl;
                }
            } else {
                cout << "[IGN ACK " << ack_seq_num << "]" << endl;
            }
        } else {
            cout << "[ERR ACK " << ack_seq_num << "]" << endl;
        }


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
    window_sent_time = new time_stamp[window_size];
    window_ack_mask = new bool[window_size];
    bool window_sent_mask[window_size];
    
    for (int i = 0; i < window_size; i++) {
        window_sent_mask[i] = false;
        window_ack_mask[i] = false;
    }

    /* Start thread to listen for ack */
    thread recv_thread(listen_ack);

    time_stamp huyu = current_time();

    lar = -1;
    lfs = lar + window_size;
    while (true) {
        /* Check window ack mask, shift window if possible */
        if (window_ack_mask[0]) {
            int shift = 0;
            for (int i = 1; i < window_size; i++) {
                shift += 1;
                if (!window_ack_mask[i]) break;
            }
            for (int i = 0; i < window_size - shift; i++) {
                window_sent_mask[i] = window_sent_mask[i + shift];
                window_ack_mask[i] = window_ack_mask[i + shift];
                window_sent_time[i] = window_sent_time[i + shift];
            }
            for (int i = window_size - shift; i < window_size; i++) {
                window_sent_mask[i] = false;
                window_ack_mask[i] = false;
            }
            lar += shift;
            lfs = lar + window_size;
        }

        for (int i = 0; i < window_size; i ++) {
            if (!window_sent_mask[i] || (!window_ack_mask[i] && elapsed_time(current_time(), window_sent_time[i]) > TIMEOUT)) {
                strcpy(data, "hello asu lu semua lkonto");
                data_size = (size_t)strlen(data);

                unsigned int seq_num = lar + i + 1;

                if (seq_num > 100) exit(1);

                frame_size = create_frame(seq_num, frame, data, data_size);

                sendto(socket_fd, frame, frame_size, MSG_CONFIRM, 
                        (const struct sockaddr *) &server_addr, sizeof(server_addr));
                window_sent_mask[i] = true;
                window_sent_time[i] = current_time();

                cout << "[SENT FRAME " << seq_num << "] " << data << endl;
            }
        }
    }

    recv_thread.join();
    delete [] window_ack_mask;
}
#include <iostream>
#include <thread>
#include <chrono>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

#include "helpers.h"

#define TIMEOUT 20

#define current_time chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define elapsed_time(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()

using namespace std;

int socket_fd;
struct sockaddr_in server_addr, client_addr;

unsigned int window_size;
bool *window_ack_mask;
time_stamp *window_sent_time;
ssize_t lar, lfs;

time_stamp TMIN = current_time();

void listen_ack() {
    char ack[ACK_SIZE];
    size_t ack_size;
    unsigned int ack_seq_num;
    bool ack_error;
    bool ack_neg;

    while (true) {
        socklen_t server_addr_size;
        ack_size = recvfrom(socket_fd, (char *)ack, ACK_SIZE, 
                MSG_WAITALL, (struct sockaddr *) &server_addr, 
                &server_addr_size);
        ack_error = read_ack(&ack_seq_num, &ack_neg, ack);

        if (!ack_error) {
            if (ack_seq_num > lar && ack_seq_num <= lfs) {
                if (!ack_neg) {
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
    char *dest_ip;
    int dest_port;
    ssize_t max_buffer_size;
    struct hostent *dest_hnet;
    char *fname;

    if (argc == 6) {
        fname = argv[1];
        window_size = atoi(argv[2]);
        max_buffer_size = atoi(argv[3]);
        dest_ip = argv[4];
        dest_port = atoi(argv[5]);
    } else {
        cerr << "usage: ./sendfile <filename> <window_size> <buffer_size> <destination_ip> <destination_port>" << endl;
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

    /* Start thread to listen for ack */
    thread recv_thread(listen_ack);

    /* Open file to send */
    FILE *file = fopen(fname, "rb");
    char *buffer;
    buffer = new char[max_buffer_size];
    size_t buffer_size;

    /* Send file */
    char frame[MAX_FRAME_SIZE];
    char data[MAX_DATA_SIZE];
    size_t frame_size;
    size_t data_size;

    while (true) {

        /* Read part of file to buffer */
        buffer_size = fread(buffer, 1, max_buffer_size, file);
        
        /* Initialize sliding window variables */
        unsigned int seq_count = buffer_size / MAX_DATA_SIZE + ((buffer_size % MAX_DATA_SIZE == 0) ? 0 : 1);
        unsigned int seq_num;
        window_sent_time = new time_stamp[window_size];
        window_ack_mask = new bool[window_size];
        bool window_sent_mask[window_size];
        for (int i = 0; i < window_size; i++) {
            window_sent_mask[i] = false;
            window_ack_mask[i] = false;
        }

        lar = -1;
        lfs = lar + window_size;
        while (true) {
            /* Check window ack mask, shift window if possible */
            if (window_ack_mask[0]) {
                unsigned int shift = 1;
                for (unsigned int i = 1; i < window_size; i++) {
                    if (!window_ack_mask[i]) break;
                    shift += 1;
                }
                for (unsigned int i = 0; i < window_size - shift; i++) {
                    window_sent_mask[i] = window_sent_mask[i + shift];
                    window_ack_mask[i] = window_ack_mask[i + shift];
                    window_sent_time[i] = window_sent_time[i + shift];
                }
                for (unsigned int i = window_size - shift; i < window_size; i++) {
                    window_sent_mask[i] = false;
                    window_ack_mask[i] = false;
                }
                lar += shift;
                lfs = lar + window_size;
            }

            for (unsigned int i = 0; i < window_size; i ++) {
                seq_num = lar + i + 1;

                if (seq_num < seq_count) {
                    if (!window_sent_mask[i] || (!window_ack_mask[i] && elapsed_time(current_time(), window_sent_time[i]) > TIMEOUT)) {
                        unsigned int buffer_shift = seq_num * MAX_DATA_SIZE;
                        data_size = (buffer_size - buffer_shift < MAX_DATA_SIZE) ? (buffer_size - buffer_shift) : MAX_DATA_SIZE;
                        memcpy(data, buffer + buffer_shift, data_size);
                        frame_size = create_frame(seq_num, frame, data, data_size);

                        sendto(socket_fd, frame, frame_size, MSG_CONFIRM, 
                                (const struct sockaddr *) &server_addr, sizeof(server_addr));
                        window_sent_mask[i] = true;
                        window_sent_time[i] = current_time();

                        cout << "[SENT FRAME " << seq_num << "] " << data << endl;
                    }
                }
            }

            if (lar >= seq_count - 1) break;
        }

        cout << "huhuhuh" << endl;
        if (buffer_size < max_buffer_size) break;
        cout << "asjkdnajsdfa" << endl;
    }
    
    cout << "harusnya kelar gan" << endl;

    recv_thread.join();
    delete [] window_ack_mask;
    delete [] window_sent_time;
}

#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAX_DATA_SIZE 1024
#define MAX_FRAME_SIZE 1034
#define ACK_SIZE 6

typedef unsigned char byte;

size_t create_frame(unsigned int seq_num, char *frame, char *data, size_t data_size);
void create_ack(unsigned int seq_num, char *ack, bool error);
bool read_frame(unsigned int *seq_num, char *data, size_t *data_size, char *frame);
bool read_ack(unsigned int *seq_num, bool *error, char *ack);

#endif
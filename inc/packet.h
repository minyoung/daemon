#ifndef PACKET_H
#define PACKET_H

typedef struct packet {
    char version;
    char type;
    short len;
    char message[1024];
} packet_t;

#endif

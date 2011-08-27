#ifndef PACKET_H
#define PACKET_H

#include <sys/types.h>

#define PACKET_HEADER_SIZE 4

typedef struct packet {
    u_int8_t version;
    int8_t type;
    u_int16_t len;
    char message[508];
} packet_t;

typedef struct stats_packet {
    u_int64_t timestamp;
    int64_t value;
    u_int8_t service_len;
    char *service;
    u_int8_t metric_len;
    char *metric;
} stats_packet_t;

#endif

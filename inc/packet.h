#ifndef PACKET_H
#define PACKET_H

#include <sys/types.h>

#define PACKET_HEADER_SIZE 4
#define MAX_TAG_COUNT 8

typedef struct packet {
    u_int8_t version;
    int8_t type;
    u_int16_t len;
    char message[508];
} packet_t;

typedef struct stats_packet {
    u_int64_t timestamp;
    int64_t value;
    char *type;
    char *service;
    char *metric;
    char *hostname;
    int tag_count;
    char **tags;
} stats_packet_t;

typedef struct log_packet {
    u_int64_t timestamp;
    char *type;
    char *service;
    char *log_line;
    char *hostname;
    int tag_count;
    char **tags;
} log_packet_t;

struct packet *packet_create();
void packet_delete(struct packet *self);
struct stats_packet *stats_packet_create(struct packet *packet);
void stats_packet_delete(struct stats_packet *self);
struct log_packet *log_packet_create(struct packet *packet);
void log_packet_delete(struct log_packet *self);

#endif

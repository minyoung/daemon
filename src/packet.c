#include "packet.h"

#include <stdlib.h>

#include "common.h"
#include "logging.h"

char *read_string(char **dest, char *buffer, int *offset) {
    char string_len = *(u_int8_t *)(&buffer[*offset]);
    *offset += 1;

    *dest = string_copy(dest, (char *)(&buffer[*offset]));
    *offset += string_len;

    return *dest;
}

struct packet *packet_create() {
    logger(stdout, LOG_DEBUG, "packet_create [%m]");
    struct packet *self = NULL;

    if (self = malloc(sizeof(*self))) {
        logger(stdout, LOG_DEBUG, "packet created");
    }

    return self;
}

void packet_delete(struct packet *self) {
    if (self != NULL) {
        free(self);
    }
}

struct stats_packet *stats_packet_create(struct packet *packet) {
    struct stats_packet *self = NULL;
    self = malloc(sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->type = NULL;
    self->service = NULL;
    self->metric = NULL;
    self->hostname = NULL;

    self->tags = malloc(MAX_TAG_COUNT * sizeof(char *));
    if (self->tags == NULL) {
        free(self);
        return NULL;
    }

    int i;
    for (i = 0; i < MAX_TAG_COUNT; ++i) {
        self->tags[i] = NULL;
    }

    int offset = 0;

    self->timestamp = *(u_int64_t *)(&packet->message[offset]);
    offset = 8;

    self->value = *(u_int64_t *)(&packet->message[offset]);
    offset += 8;

    read_string(&self->service, packet->message, &offset);
    read_string(&self->metric, packet->message, &offset);
    read_string(&self->hostname, packet->message, &offset);

    i = 0;
    while (offset < packet->len) {
        read_string(&self->tags[i], packet->message, &offset);
        i += 1;
    }
    self->tag_count = i;

    return self;
}

void stats_packet_delete(struct stats_packet *self) {
    if (self != NULL) {
        free(self->type);
        free(self->service);
        free(self->metric);
        free(self->hostname);
        int i = 0;
        for (i = 0; i < self->tag_count; i++) {
            free(self->tags[i]);
        }
        free(self->tags);
        free(self);
    }
}

struct log_packet *log_packet_create(struct packet *packet) {
    struct log_packet *self = NULL;
    self = malloc(sizeof(*self));
    if (self == NULL) {
        return NULL;
    }

    self->type = NULL;
    self->service = NULL;
    self->log_line = NULL;
    self->hostname = NULL;
    self->tags = malloc(MAX_TAG_COUNT * sizeof(char *));
    if (self->tags == NULL) {
        free(self);
        return NULL;
    }

    int i;
    for (i = 0; i < MAX_TAG_COUNT; ++i) {
        self->tags[i] = NULL;
    }

    int offset = 0;

    self->timestamp = *(u_int64_t *)(&packet->message[offset]);
    offset = 8;

    read_string(&self->service, packet->message, &offset);
    read_string(&self->log_line, packet->message, &offset);
    read_string(&self->hostname, packet->message, &offset);

    i = 0;
    while (offset < packet->len) {
        read_string(&self->tags[i], packet->message, &offset);
        i += 1;
    }
    self->tag_count = i;

    return self;
}

void log_packet_delete(struct log_packet *self) {
    if (self != NULL) {
        free(self->type);
        free(self->service);
        free(self->log_line);
        free(self->hostname);
        int i = 0;
        for (i = 0; i < self->tag_count; i++) {
            free(self->tags[i]);
        }
        free(self->tags);
        free(self);
    }
}

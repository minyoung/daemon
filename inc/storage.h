#ifndef STORAGE_H
#define STORAGE_H

#define STORAGE_VERSION "S0"

#include "common.h"
#include "daemon.h"
#include "packet.h"

char *storage_format_data_filename(struct daemon *self, unsigned char *hash);
unsigned char *storage_get_stats_hash(struct daemon *self, struct stats_packet *packet);
status_t storage_store_stats(struct daemon *self, struct stats_packet *packet);

#endif

#ifndef STORAGE_H
#define STORAGE_H

#include "common.h"
#include "daemon.h"
#include "packet.h"

unsigned char *storage_get_stats_hash(struct daemon *self, struct stats_packet *packet);
status_t storage_store_stats(struct daemon *self, struct stats_packet *packet);

#endif

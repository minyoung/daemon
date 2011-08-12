#ifndef NETWORK_H
#define NETWORK_H

#include "daemon.h"

void network_close_socket(struct daemon *self);
status_t network_open_socket(struct daemon *self);
void network_handle_socket(struct daemon *self);

#endif

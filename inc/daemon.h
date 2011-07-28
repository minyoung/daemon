#ifndef DAEMON_H
#define DAEMON_H

#include <unistd.h>

typedef struct daemon {
    pid_t pid;
} daemon_t;

struct daemon *daemon_new();
void daemon_delete(struct daemon *self);

#endif

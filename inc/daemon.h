#ifndef DAEMON_H
#define DAEMON_H

#include <unistd.h>

#include "config.h"

typedef struct daemon {
    struct config *config;
    pid_t pid;

    FILE *log_file;
    int lock_fd;

    int server_socket;
} daemon_t;

typedef int daemonize_status_t;
const daemonize_status_t DAEMONIZE_SUCCESS;
const daemonize_status_t DAEMONIZE_FAILURE;
const daemonize_status_t DAEMON_DAEMONIZED;

struct daemon *daemon_new(struct config *config);
void daemon_delete(struct daemon *self);

daemonize_status_t daemon_daemonize(struct daemon *self);
void daemon_run(struct daemon *self);

#endif

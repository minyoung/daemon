#include "daemon.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logging.h"

const daemonize_status_t DAEMONIZE_SUCCESS = 0;
const daemonize_status_t DAEMONIZE_FAILURE = 1;
const daemonize_status_t DAEMON_DAEMONIZED = 2;

const int DAEMON_CONTROL = 0;
const int DAEMON_CLIENT = 1;

void daemon_log(struct daemon *self, int priority, char *format, ...);

struct daemon *daemon_create(struct config *config) {
    logger(stdout, LOG_DEBUG, "daemon_create [%m]");
    struct daemon *self = NULL;

    if (self = malloc(sizeof(*self))) {
        self->config = config;
        if (self->config->log_filename != NULL) {
            self->log_file = fopen(self->config->log_filename, "a");
        } else {
            self->log_file = NULL;
        }
        self->lock_fd = 0;

        self->network_sockets[0] = 0;
        self->network_sockets[1] = 0;

        self->network_threads[0] = 0;
        self->network_threads[1] = 0;

        self->running = 1;
        daemon_log(self, LOG_DEBUG, "daemon created [%m]");
    }

    return self;
}

void daemon_delete(struct daemon *self) {
    if (self != NULL) {
        network_close_sockets(self);
        config_delete(self->config);
        self->config = NULL;

        if (self->log_file != NULL) {
            fclose(self->log_file);
            self->log_file = NULL;
        }

        if (self->lock_fd != 0) {
            close(self->lock_fd);
            self->lock_fd = 0;
        }

        free(self);
    }
}

void daemon_exit(struct daemon *self, int exit_code) {
    daemon_delete(self);
    exit(exit_code);
}

void daemon_err(struct daemon *self, int priority, char *format, ...) {
    va_list args;
    if (priority > self->config->log_level) {
        return;
    }

    va_start(args, format);
    vlogger(stderr, priority, format, args);
    if (self->log_file != NULL) {
        vlogger(self->log_file, priority, format, args);
    }
}

void daemon_log(struct daemon *self, int priority, char *format, ...) {
    va_list args;
    if (priority > self->config->log_level) {
        return;
    }

    va_start(args, format);
    vlogger(stdout, priority, format, args);
    if (self->log_file != NULL) {
        vlogger(self->log_file, priority, format, args);
    }
}


status_t daemon_get_lock(struct daemon *self) {
    self->lock_fd = open(self->config->lock_filename, O_RDWR | O_CREAT, 0640);

    if (self->lock_fd == -1) {
        return FAILURE;
    }

    struct flock lock = {
        .l_type = F_WRLCK,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_len = 0,
        .l_pid = 0
    };

    daemon_log(self, LOG_INFO, "Trying to get lock on file: \"%s\"", self->config->lock_filename);
    if (fcntl(self->lock_fd, F_SETLK, &lock) == -1) {
        daemon_log(self, LOG_ERR, "Could not get lock on file \"%s\" [%m]", self->config->lock_filename);
        daemon_log(self, LOG_INFO, "Is the daemon already running?");
        return FAILURE;
    }

    if (ftruncate(self->lock_fd, 0) != 0) {
        daemon_log(self, LOG_ERR, "Could not truncate lock file for writing [%m]");
        return FAILURE;
    }

    char pidtext[32];
    snprintf(pidtext, sizeof(pidtext), "%d\n", self->pid);

    if (write(self->lock_fd, pidtext, strlen(pidtext)) < 0) {
        daemon_log(self, LOG_ERR, "Could not write pid (%d) to lock [%m]", self->pid);
        return FAILURE;
    }

    daemon_log(self, LOG_INFO, "Opened lock and wrote pid (%d)", self->pid);

    return SUCCESS;
}

daemonize_status_t daemon_daemonize(struct daemon *self) {
    if (self->config->daemon == 0) {
        daemon_log(self, LOG_INFO, "Not daemonizing");
        self->pid = getpid();
        return DAEMON_DAEMONIZED;
    }

    if ((self->pid = fork()) < 0) {
        daemon_log(self, LOG_CRIT, "Could not fork off child [%m]");
        return DAEMONIZE_FAILURE;
    } else if (self->pid != 0) {
        logger(stdout, LOG_NOTICE, "Child daemon (%d) forked off [%m]", self->pid);
        return DAEMONIZE_SUCCESS;
    }

    self->pid = getpid();
    daemon_log(self, LOG_INFO, "Child created: %d", self->pid);

    /* umask(027); */
    if (setsid() < 0) {
        daemon_log(self, LOG_CRIT, "Could not set session id [%m]");
        daemon_exit(self, 1);
    }
}

void daemon_run(struct daemon *self) {
    if (daemon_get_lock(self) == FAILURE) {
        logger(stderr, LOG_ERR, "Could not get lock on file (%s) [%m]", self->config->lock_filename);
        return;
    }

    if (network_open_sockets(self) == FAILURE) {
        daemon_err(self, LOG_ERR, "Error creating server socket [%m]");
        return;
    }

    if (network_start_threads(self) == FAILURE) {
        daemon_err(self, LOG_ERR, "Could not start network threads [%m]");
        return;
    }

    daemon_log(self, LOG_NOTICE, "Daemon started [%m]");

    pthread_join(self->network_threads[DAEMON_CONTROL], NULL);
    pthread_join(self->network_threads[DAEMON_CLIENT], NULL);
}

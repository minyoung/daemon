#ifndef config_H
#define config_H

#include "common.h"

#define DEFAULT_CONFIG_FILE "daemon.conf"
#define DEFAULT_LOCK_FILE "daemon.pid"
#define DEFAULT_LOG_FILE "daemon.log"
#define DEFAULT_LOG_LEVEL LOG_NOTICE

typedef struct config {
    int daemon;
    int log_level;
    char *config_filename;
    char *log_filename;
    char *lock_filename;
} config_t;

struct config *config_new();
void config_delete(struct config *self);

status_t config_parse_args(struct config *self, int argc, char **argv);
status_t config_load_file(struct config *self, const char *filename);

#endif

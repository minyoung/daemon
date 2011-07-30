#ifndef config_H
#define config_H

#include "common.h"

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

#endif

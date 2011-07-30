#include "daemon.h"

#include <stdlib.h>

#include "common.h"
#include "logging.h"
#include "unit_testing.h"

struct daemon *daemon_new() {
    logger(stdout, LOG_DEBUG, "daemon_new [%m]");
    struct daemon *self = NULL;

    if (self = malloc(sizeof(*self))) {
        logger(stdout, LOG_DEBUG, "daemon created");
    }

    return self;
}

void daemon_delete(struct daemon *self) {
    if (self != NULL) {
        free(self);
    }
}

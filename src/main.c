#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "daemon.h"
#include "logging.h"

void cleanup(struct config *config, struct daemon *daemon) {
    config_delete(config);
    daemon_delete(daemon);
}

int main(int argc, char **argv) {
    struct config *config = NULL;
    struct daemon *daemon = NULL;
    status_t return_code = SUCCESS;

    if ((config = config_new()) == NULL) {
        logger(stderr, LOG_CRIT, "Could not create config object [%m]");
        cleanup(config, daemon);
        exit(FAILURE);
    }

    return_code = config_parse_args(config, argc, argv);
    if (return_code != SUCCESS) {
        cleanup(config, daemon);
        exit(return_code);
    }

    if ((daemon = daemon_new(config)) == NULL) {
        logger(stderr, LOG_CRIT, "Could not create daemon object [%m]");
        cleanup(config, daemon);
        exit(FAILURE);
    }

    cleanup(config, daemon);
    logger(stdout, LOG_INFO, "Finish");
    return 0;
}

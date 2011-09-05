#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "daemon.h"
#include "logging.h"

void cleanup(struct config *config, struct daemon *daemon) {
    logger(stdout, LOG_DEBUG, "Cleaning up [%m]");
    if (daemon != NULL) {
        daemon_delete(daemon);
    } else {
        config_delete(config);
    }
}

int main(int argc, char **argv) {
    struct config *config = NULL;
    struct daemon *daemon = NULL;
    status_t return_code = SUCCESS;

    config = config_create();
    if (config == NULL) {
        logger(stderr, LOG_CRIT, "Could not create config object [%m]");
        cleanup(config, daemon);
        exit(FAILURE);
    }

    return_code = config_parse_args(config, argc, argv);
    if (return_code != SUCCESS) {
        cleanup(config, daemon);
        exit(return_code);
    }

    return_code = config_load_file(config, config->config_filename);
    if (return_code != SUCCESS) {
        logger(stderr, LOG_ERR, "Error loading config file [%m]");
        cleanup(config, daemon);
        exit(return_code);
    }

    daemon = daemon_create(config);
    if (daemon == NULL) {
        logger(stderr, LOG_CRIT, "Could not create daemon object [%m]");
        cleanup(config, daemon);
        exit(FAILURE);
    }

    return_code = daemon_daemonize(daemon);
    if (return_code == DAEMON_DAEMONIZED) {
        daemon_run(daemon);
    }

    daemon_delete(daemon);
    logger(stdout, LOG_INFO, "Finish");
    return 0;
}

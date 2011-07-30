#include <stdlib.h>

#include "common.h"
#include "config.h"
#include "daemon.h"
#include "logging.h"

int main(int argc, char **argv) {
    struct config *config = NULL;
    struct daemon *daemon = NULL;
    status_t return_code = SUCCESS;

    if ((config = config_new()) == NULL) {
        logger(stderr, LOG_CRIT, "Could not create config object [%m]");
        exit(FAILURE);
    }

    return_code = config_parse_args(config, argc, argv);
    if (return_code != SUCCESS) {
        exit(return_code);
    }

    if ((daemon = daemon_new(config)) == NULL) {
        logger(stderr, LOG_CRIT, "Could not create daemon object [%m]");
        exit(FAILURE);
    }

    config_delete(config);
    daemon_delete(daemon);

    logger(stdout, LOG_INFO, "Finish");
    return 0;
}

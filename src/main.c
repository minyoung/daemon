#include <stdlib.h>

#include "common.h"
#include "daemon.h"
#include "logging.h"
#include "unit_testing.h"

int main(int argc, char **argv) {
    struct daemon *daemon = NULL;
    if ((daemon = daemon_new()) == NULL) {
        exit(FAILURE);
    }

    daemon_delete(daemon);

    logger(stdout, LOG_INFO, "Finish");
    return 0;
}

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "logging.h"

static const char *const args_short= "hsc:df:l:v";
static const struct option args_long[] = {
    { "help",        0, NULL, 'h' },
    { "show",        0, NULL, 's' },
    { "config",      1, NULL, 'c' },
    { "daemon",      0, NULL, 'd' },
    { "flock",       1, NULL, 'f' },
    { "log",         1, NULL, 'l' },
    { "verbose",     0, NULL, 'v' },
    { 0,             0, NULL,  0  }
};

struct config *config_new() {
    logger(stdout, LOG_DEBUG, "config_new [%m]");
    struct config *self = NULL;

    if (self = malloc(sizeof(*self))) {
        logger(stdout, LOG_DEBUG, "config created");
    }

    return self;
}

void config_delete(struct config *self) {
    if (self != NULL) {
        free(self);
    }
}


void config_print_usage(struct config *self, FILE *stream) {
    fprintf(stream,
            "Usage: daemon arg_config\n"
            "  --help           -h      Display this help screen\n"
            "  --show           -s      Show config config\n"
            "  --config=arg     -c      Specify config file to load\n"
            "                               Default: none\n"
            "  --daemon         -d      Run as a daemon\n"
            "                               Default: false\n"
            "  --flock=arg      -f      Specify lock/pid file\n"
            "                               Default: statsd.pid\n"
            "  --log=arg        -l      Specify log file\n"
            "                               Default: statsd.log\n"
            "  --verbose        -v      Output verbose information\n"
            "                               Default: false\n"
    );
}

status_t config_parse_args(struct config *self, int argc, char **argv) {
    int next = 0;
    int index = 0;

    do {
        next = getopt_long(argc, argv, args_short, args_long, &index);
        switch (next) {
            case 's': // show
                config_print_usage(self, stdout);
                return FAILURE;
            case 'c': // config file
                string_copy(&(self->config_filename), optarg);
                logger(stdout, LOG_DEBUG, "Setting config file: %s", self->config_filename);
                break;
            case 'd': // daemon
                self->daemon = 1;
                break;
            case 'f': // lock file
                string_copy(&(self->lock_filename), optarg);
                logger(stdout, LOG_DEBUG, "Setting lock file: %s", self->lock_filename);
                break;
            case 'l': // log file
                string_copy(&(self->log_filename), optarg);
                logger(stdout, LOG_DEBUG, "Setting log file: %s", self->log_filename);
                break;
            case 'v': //verbose
                self->log_level = LOG_DEBUG;
                break;
            case -1:
                break;
            case '?':
            case ':':
                config_print_usage(self, stdout);
                return FAILURE;
            case 'h':
            default:
                config_print_usage(self, stdout);
                return SUCCESS;
        }
    } while (next != -1);

    /* if (self->config_filename != NULL) { */
        /* configuration_load_from_file(daemonizer); */
    /* } */
    /* copy_if_null(&(self->config_filename), "statsd.conf"); */

/* #define copy_if_null(dest, value) if (dest == NULL) string_copy(&(dest), value); */
    copy_if_null(&(self->lock_filename), "daemon.pid");
    copy_if_null(&(self->log_filename), "daemon.log");

    return SUCCESS;
}

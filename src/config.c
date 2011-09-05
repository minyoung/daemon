#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <ini.h>

#include "logging.h"

static const char *const args_short= "hsc:df:l:p:o:v";
static const struct option args_long[] = {
    { "help",        0, NULL, 'h' },
    { "show",        0, NULL, 's' },
    { "config",      1, NULL, 'c' },
    { "daemon",      0, NULL, 'd' },
    { "flock",       1, NULL, 'f' },
    { "log",         1, NULL, 'l' },
    { "port",        1, NULL, 'p' },
    { "control",     1, NULL, 'o' },
    { "verbose",     0, NULL, 'v' },
    { 0,             0, NULL,  0  }
};

struct config *config_create() {
    logger(stdout, LOG_DEBUG, "config_create [%m]");
    struct config *self = NULL;

    if (self = malloc(sizeof(*self))) {
        logger(stdout, LOG_DEBUG, "config created");
        self->log_level = LOG_DEBUG;
        self->config_filename = NULL;
        self->log_filename = NULL;
        self->lock_filename = NULL;
        self->client_port = NULL;
        self->control_port = NULL;
    }

    return self;
}

void config_delete(struct config *self) {
    if (self != NULL) {
        free_pointer(&self->config_filename);
        free_pointer(&self->log_filename);
        free_pointer(&self->lock_filename);
        free_pointer(&self->client_port);
        free_pointer(&self->control_port);
        free(self);
    }
}


void config_print_usage(struct config *self, FILE *stream) {
    fprintf(stream,
            "Usage: daemon arg_config\n"
            "  --help           -h      Display this help screen\n"
            "  --show           -s      Show config config\n"
            "  --daemon         -d      Run as a daemon\n"
            "                               Default: %s\n"
            "  --config=arg     -c      Specify config file to load\n"
            "                               Default: %s\n"
            "  --flock=arg      -f      Specify lock/pid file\n"
            "                               Default: %s\n"
            "  --log=arg        -l      Specify log file\n"
            "                               Default: %s\n"
            "  --port=arg       -p      Specify client port number\n"
            "                               Default: %s\n"
            "  --control=arg    -o      Specify server control port number\n"
            "                               Default: %s\n"
            "  --verbose        -v      Output verbose information\n"
            "                               Default: %s\n",
        "false",
        DEFAULT_CONFIG_FILE,
        DEFAULT_LOCK_FILE,
        DEFAULT_LOG_FILE,
        DEFAULT_CLIENT_PORT,
        DEFAULT_CONTROL_PORT,
        "false"
    );
}

status_t config_parse_args(struct config *self, int argc, char **argv) {
    int next = 0;
    int index = 0;

    // reset option parser (mainly for testing...)
    optind = 1;
    self->daemon = 0;
    string_copy(&self->config_filename, DEFAULT_CONFIG_FILE);
    string_copy(&self->lock_filename, DEFAULT_LOCK_FILE);
    string_copy(&self->log_filename, DEFAULT_LOG_FILE);
    self->log_level = DEFAULT_LOG_LEVEL;
    string_copy(&self->client_port, DEFAULT_CLIENT_PORT);
    string_copy(&self->control_port, DEFAULT_CONTROL_PORT);

    do {
        next = getopt_long(argc, argv, args_short, args_long, &index);
        switch (next) {
            case 's': // show
                /* config_print_usage(self, stdout); */
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
            case 'p': // client port
                string_copy(&(self->client_port), optarg);
                logger(stdout, LOG_DEBUG, "Setting client port: %s", self->client_port);
            case 'o': // control port
                string_copy(&(self->control_port), optarg);
                logger(stdout, LOG_DEBUG, "Setting control port: %s", self->control_port);
            case 'l': // log file
                string_copy(&(self->log_filename), optarg);
                logger(stdout, LOG_DEBUG, "Setting log file: %s", self->log_filename);
                break;
            case 'v': // verbose
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
                return FAILURE;
        }
    } while (next != -1);
    return SUCCESS;
}

int config_ini_handler(void *user, const char *section, const char *name, const char *value) {
    struct config *self = (struct config*)user;
/* #define MATCH(s, n) stricmp(section, s) == 0 && stricmp(name, n) == 0 */
#define MATCH(n) strcasecmp(name, n) == 0
    if (MATCH("daemon")) {
        self->daemon = atoi(value) == 0 ? 0 : 1;
    } else if (MATCH("verbose")) {
        if (atoi(value) != 0) {
            self->log_level = LOG_DEBUG;
        }
    } else if (MATCH("port")) {
        string_copy(&(self->client_port), value);
        logger(stdout, LOG_DEBUG, "Setting client port: %s", self->client_port);
    } else if (MATCH("control")) {
        string_copy(&(self->control_port), value);
        logger(stdout, LOG_DEBUG, "Setting control port: %s", self->control_port);
    } else if (MATCH("lock")) {
        string_copy(&(self->lock_filename), value);
        logger(stdout, LOG_DEBUG, "Setting lock file: %s", self->lock_filename);
    } else if (MATCH("log")) {
        string_copy(&(self->log_filename), value);
        logger(stdout, LOG_DEBUG, "Setting log file: %s", self->log_filename);
    }
#undef MATCH
    return 1;
}

status_t config_load_file(struct config *self, const char *filename) {
    if (filename == NULL) {
        return SUCCESS;
    }
    return ini_parse(filename, config_ini_handler, self) == 0 ? SUCCESS : FAILURE;
}

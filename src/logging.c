#include <stdlib.h>
#include <syslog.h>
#include <time.h>

#include "logging.h"

void vlogger(FILE *stream, int priority, char *format, va_list args) {
    char time_string[32];
    char *log_level;
    time_t current_time = time(NULL);
    struct tm *time_value = gmtime(&current_time);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S UTC", time_value);
    switch (priority) {
        case LOG_EMERG:
            log_level = "EMERGENCY";
            break;
        case LOG_ALERT:
            log_level = "ALERT";
            break;
        case LOG_CRIT:
            log_level = "CRITICAL";
            break;
        case LOG_ERR:
            log_level = "ERROR";
            break;
        case LOG_WARNING:
            log_level = "WARNING";
            break;
        case LOG_NOTICE:
            log_level = "NOTICE";
            break;
        case LOG_INFO:
            log_level = "INFO";
            break;
        case LOG_DEBUG:
            log_level = "DEBUG";
            break;
    }

    fprintf(stream, "%s [%s]: ", time_string, log_level);
    vfprintf(stream, format, args);
    fprintf(stream, "\n");
}

void logger(FILE *stream, int priority, char *format, ...) {
    va_list args;
    va_start(args, format);
    vlogger(stream, priority, format, args);
}


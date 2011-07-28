#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

void vlogger(FILE *stream, int priority, char *format, va_list args);
void logger(FILE *stream, int priority, char *format, ...);

#endif

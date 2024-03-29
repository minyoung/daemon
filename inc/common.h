#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

typedef int status_t;

const status_t SUCCESS;
const status_t FAILURE;

char *string_copy(char **dest, const char *src);

char *copy_if_null(char **dest, char *value);
void free_pointer(char **ptr);
inline void close_file(FILE **file);

#endif

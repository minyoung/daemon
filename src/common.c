#include "common.h"

#include <string.h>
#include <stdlib.h>

const status_t SUCCESS = 0;
const status_t FAILURE = 1;

char *string_copy(char **dest, const char *src) {
    if (dest == NULL) {
        return strdup(src);
    }

    if (*dest != NULL) {
        free(*dest);
    }
    *dest = strdup(src);
    return *dest;
}

/* conditionally copy value into dest if dest is null */
inline char *copy_if_null(char **dest, char *value) {
    if (*dest == NULL) {
        return string_copy(dest, value);
    }
    return NULL;
}

/* free and reset the pointer if it is not null */
inline void free_pointer(char **ptr) {
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

inline void close_file(FILE **file) {
    if (*file != NULL) {
        fclose(*file);
        *file = NULL;
    }
}

#include <assert.h>
#include <string.h>
#include "unit_testing.h"

#include "logging.h"

void test_logging_level(int priority, char *message, char *expected) {
    int fd[2];
    char str[64];
    FILE *output_stream;
    assert(pipe(fd) == 0);
    output_stream = fdopen(fd[1], "w");
    /*
     * fdopen: fd -> stream
     * fileno: stream -> fd
     */

    logger(output_stream, priority, message);
    fclose(output_stream);

    assert(read(fd[0], str, sizeof(str)) > 0);
    /* assert(strncmp(str, "yyyy-mm-dd HH:MM:SS UTC [DEBUG]: test")); */
    assert(strncmp(&str[24], expected, strlen(expected)) == 0);

    close(fd[0]);
    close(fd[1]);
}

void test_basic_logging(void **state) {
    test_logging_level(LOG_DEBUG,   "test", "[DEBUG]: test");
    test_logging_level(LOG_INFO,    "test", "[INFO]: test");
    test_logging_level(LOG_NOTICE,  "test", "[NOTICE]: test");
    test_logging_level(LOG_WARNING, "test", "[WARNING]: test");
    test_logging_level(LOG_ERR,     "test", "[ERROR]: test");
    test_logging_level(LOG_CRIT,    "test", "[CRITICAL]: test");
    test_logging_level(LOG_ALERT,   "test", "[ALERT]: test");
    test_logging_level(LOG_EMERG,   "test", "[EMERGENCY]: test");
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_basic_logging),
    };
    return run_tests(tests);
}

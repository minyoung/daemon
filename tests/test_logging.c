#include <assert.h>
#include <string.h>
#include "unit_testing.h"

#include "logging.h"

void test_basic_logging(void **state) {
    int fd[2];
    char str[38];
    FILE *output_stream;
    assert(pipe(fd) == 0);
    output_stream = fdopen(fd[1], "w");
    /*
     * fdopen: fd -> stream
     * fileno: stream -> fd
     */

    logger(output_stream, LOG_DEBUG, "test");
    fclose(output_stream);

    assert(read(fd[0], str, sizeof(str)) > 0);
    /* assert(strncmp(str, "yyyy-mm-dd HH:MM:SS UTC [DEBUG]: test")); */
    assert(strncmp(&str[24], "[DEBUG]: test", 13) == 0);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_basic_logging),
    };
    return run_tests(tests);
}

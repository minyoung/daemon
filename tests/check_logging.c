#include "logging.h"
#include "checkhelper.c"

void test_logging_level(int priority, char *message, char *expected) {
    int fd[2];
    char str[64];
    FILE *output_stream;
    ck_assert(pipe(fd) == 0);
    output_stream = fdopen(fd[1], "w");
    /*
     * fdopen: fd -> stream
     * fileno: stream -> fd
     */

    logger(output_stream, priority, message);
    fclose(output_stream);

    ck_assert(read(fd[0], str, sizeof(str)) > 0);
    /* ck_assert(strncmp(str, "yyyy-mm-dd HH:MM:SS UTC [DEBUG]: test")); */
    ck_assert(strncmp(&str[24], expected, strlen(expected)) == 0);

    close(fd[0]);
    close(fd[1]);
}

START_TEST (check_basic_logging) {
    test_logging_level(LOG_DEBUG,   "test", "[DEBUG]: test");
    test_logging_level(LOG_INFO,    "test", "[INFO]: test");
    test_logging_level(LOG_NOTICE,  "test", "[NOTICE]: test");
    test_logging_level(LOG_WARNING, "test", "[WARNING]: test");
    test_logging_level(LOG_ERR,     "test", "[ERROR]: test");
    test_logging_level(LOG_CRIT,    "test", "[CRITICAL]: test");
    test_logging_level(LOG_ALERT,   "test", "[ALERT]: test");
    test_logging_level(LOG_EMERG,   "test", "[EMERGENCY]: test");
}
END_TEST

Suite *check_suite(void) {
    Suite *s = suite_create("check logging");

    TCase *tc_core = tcase_create("Logging");
    tcase_add_test(tc_core, check_basic_logging);
    suite_add_tcase(s, tc_core);

    return s;
}

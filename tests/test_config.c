#include <assert.h>
#include "unit_testing.h"

#include "config.h"

#include "logging.h"

void test_config_new_and_delete_does_not_leak_memory(void **state) {
    struct config *config = NULL;
    config = config_new();
    config_delete(config);
}

void test_config_parse_args(void **state) {
    int argc = 9;
    char *argv[] = {
        "test_config",
        "-d",
        "-v",
        "--config", "config_filename",
        "--flock", "lock_filename",
        "--log", "log_filename"
    };
    struct config *self = config_new();

    assert_int_equal(config_parse_args(self, argc, argv), SUCCESS);
    assert_int_equal(self->daemon, 1);
    assert_int_equal(self->log_level, LOG_DEBUG);
    assert_string_equal(self->config_filename, "config_filename");
    assert_string_equal(self->lock_filename, "lock_filename");
    assert_string_equal(self->log_filename, "log_filename");

    config_delete(self);
}

void test_config_default_args(void **state) {
    int argc = 1;
    // have an extra NULL arg, otherwise this seg faults while testing...
    char *argv[] = {
        "test_config", 0
    };
    struct config *self = config_new();

    assert_int_equal(config_parse_args(self, argc, argv), SUCCESS);
    assert_int_equal(self->daemon, 0);
    assert_int_equal(self->log_level, LOG_NOTICE);
    assert_string_equal(self->config_filename, "daemon.conf");
    assert_string_equal(self->lock_filename, "daemon.pid");
    assert_string_equal(self->log_filename, "daemon.log");

    config_delete(self);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_config_new_and_delete_does_not_leak_memory),
        unit_test(test_config_parse_args),
        unit_test(test_config_default_args),
    };
    return run_tests(tests);
}


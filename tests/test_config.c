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
    int argc = 11;
    char *argv[] = {
        "test_config",
        "-d",
        "-v",
        "--config", "config_filename",
        "--port", "7357",
        "--flock", "lock_filename",
        "--log", "log_filename"
    };
    struct config *self = config_new();

    assert_int_equal(config_parse_args(self, argc, argv), SUCCESS);
    assert_int_equal(self->daemon, 1);
    assert_int_equal(self->log_level, LOG_DEBUG);
    assert_string_equal(self->config_filename, "config_filename");
    assert_string_equal(self->port, "7357");
    assert_string_equal(self->lock_filename, "lock_filename");
    assert_string_equal(self->log_filename, "log_filename");

    config_delete(self);
}

void test_config_default_args(void **state) {
    int argc = 1;
    char *argv[] = {
        "test_config"
    };
    struct config *self = config_new();

    assert_int_equal(config_parse_args(self, argc, argv), SUCCESS);
    assert_int_equal(self->daemon, 0);
    assert_int_equal(self->log_level, LOG_NOTICE);
    assert_string_equal(self->config_filename, "daemon.conf");
    assert_string_equal(self->port, "7357");
    assert_string_equal(self->lock_filename, "daemon.pid");
    assert_string_equal(self->log_filename, "daemon.log");

    config_delete(self);
}

void test_config_load_file(void **state) {
    struct config *self = config_new();

    assert_int_equal(config_load_file(self, "../tests/test.conf"), 0);
    assert_int_equal(self->daemon, 1);
    assert_int_equal(self->log_level, LOG_DEBUG);
    assert_string_equal(self->lock_filename, "lock_file");
    assert_string_equal(self->log_filename, "log_file");

    config_delete(self);
}

void test_config_load_blank_file(void **state) {
    struct config *self = config_new();
    int argc = 1;
    char *argv[] = {
        "test_config"
    };

    assert_int_equal(config_parse_args(self, argc, argv), SUCCESS);
    assert_int_equal(config_load_file(self, "../tests/blank.conf"), 0);
    assert_int_equal(self->daemon, 0);
    assert_int_equal(self->log_level, LOG_NOTICE);
    assert_string_equal(self->lock_filename, "daemon.pid");
    assert_string_equal(self->log_filename, "daemon.log");

    config_delete(self);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_config_new_and_delete_does_not_leak_memory),
        unit_test(test_config_parse_args),
        unit_test(test_config_default_args),
        unit_test(test_config_load_file),
        unit_test(test_config_load_blank_file),
    };
    return run_tests(tests);
}


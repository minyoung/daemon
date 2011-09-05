#include "config.h"
#include "checkhelper.c"

#include "logging.h"

START_TEST (check_config_create_and_delete) {
    struct config *config = NULL;
    config = config_create();
    config_delete(config);
}
END_TEST

START_TEST (check_config_parse_args) {
    int argc = 13;
    char *argv[] = {
        "test_config",
        "-d",
        "-v",
        "--config", "config_filename",
        "--port", "7357",
        "--control", "7358",
        "--flock", "lock_filename",
        "--log", "log_filename"
    };
    struct config *self = config_create();

    ck_assert(config_parse_args(self, argc, argv) == SUCCESS);
    ck_assert_int_eq(self->daemon, 1);
    ck_assert_int_eq(self->log_level, LOG_DEBUG);
    ck_assert_str_eq(self->config_filename, "config_filename");
    ck_assert_str_eq(self->client_port, "7357");
    ck_assert_str_eq(self->control_port, "7358");
    ck_assert_str_eq(self->lock_filename, "lock_filename");
    ck_assert_str_eq(self->log_filename, "log_filename");

    config_delete(self);
}
END_TEST

START_TEST (check_config_default_args) {
    int argc = 1;
    char *argv[] = {
        "test_config"
    };
    struct config *self = config_create();

    ck_assert(config_parse_args(self, argc, argv) == SUCCESS);
    ck_assert_int_eq(self->daemon, 0);
    ck_assert_int_eq(self->log_level, LOG_NOTICE);
    ck_assert_str_eq(self->config_filename, "daemon.conf");
    ck_assert_str_eq(self->client_port, "10180");
    ck_assert_str_eq(self->control_port, "10181");
    ck_assert_str_eq(self->lock_filename, "daemon.pid");
    ck_assert_str_eq(self->log_filename, "daemon.log");

    config_delete(self);
}
END_TEST

START_TEST (check_config_load_file) {
    struct config *self = config_create();

    ck_assert(config_load_file(self, "../tests/test.conf") == 0);
    ck_assert_int_eq(self->daemon, 1);
    ck_assert_int_eq(self->log_level, LOG_DEBUG);
    ck_assert_str_eq(self->lock_filename, "lock_file");
    ck_assert_str_eq(self->log_filename, "log_file");
    ck_assert_str_eq(self->client_port, "7357");
    ck_assert_str_eq(self->control_port, "7358");

    config_delete(self);
}
END_TEST

START_TEST (check_config_load_blank_file) {
    struct config *self = config_create();
    int argc = 1;
    char *argv[] = {
        "test_config"
    };

    ck_assert(config_parse_args(self, argc, argv) == SUCCESS);
    ck_assert_int_eq(config_load_file(self, "../tests/blank.conf"), 0);
    ck_assert_int_eq(self->daemon, 0);
    ck_assert_int_eq(self->log_level, LOG_NOTICE);
    ck_assert_str_eq(self->lock_filename, "daemon.pid");
    ck_assert_str_eq(self->log_filename, "daemon.log");
    ck_assert_str_eq(self->client_port, "10180");
    ck_assert_str_eq(self->control_port, "10181");

    config_delete(self);
}
END_TEST

Suite *check_suite(void) {
    Suite *s = suite_create("check config");

    TCase *tc_core = tcase_create("Config");
    tcase_add_test(tc_core, check_config_create_and_delete);
    tcase_add_test(tc_core, check_config_parse_args);
    tcase_add_test(tc_core, check_config_default_args);
    tcase_add_test(tc_core, check_config_load_file);
    tcase_add_test(tc_core, check_config_load_blank_file);
    suite_add_tcase(s, tc_core);

    return s;
}

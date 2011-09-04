#include "daemon.h"
#include "checkhelper.c"

#include <fcntl.h>

START_TEST (check_daemon_new_and_delete) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    daemon = daemon_new(config);
    daemon_delete(daemon);
}
END_TEST

START_TEST (check_daemon_can_flock_a_file) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->lock_filename = "test.lock";
    daemon = daemon_new(config);

    ck_assert(daemon_get_lock(daemon) == SUCCESS);

    daemon_delete(daemon);
}
END_TEST

START_TEST (check_daemon_fails_to_flock_an_already_flocked_file) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->lock_filename = "test.lock";
    daemon = daemon_new(config);

    int to_parent[2];
    int to_child[2];
    ck_assert(pipe(to_parent) == 0);
    ck_assert(pipe(to_child) == 0);
    int lock_fd = 0;
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_len = 0,
        .l_pid = 1
    };
    pid_t pid = fork();
    ck_assert(pid >= 0);
    char buf[1];

    if (pid == 0) {
        // parent
        read(to_parent[0], buf, 1);
        ck_assert(daemon_get_lock(daemon) == FAILURE);
        write(to_child[1], buf, 1);
        read(to_parent[0], buf, 1);
        ck_assert(daemon_get_lock(daemon) == SUCCESS);
    } else {
        // child
        lock_fd = open(config->lock_filename, O_RDWR | O_CREAT, 0640);
        ck_assert_int_ne(lock_fd, -1);
        ck_assert(fcntl(lock_fd, F_SETLK, &lock) != -1);
        write(to_parent[1], buf, 1);
        read(to_child[0], buf, 1);
        lock.l_type = F_UNLCK;
        ck_assert(fcntl(lock_fd, F_SETLK, &lock) != -1);
        write(to_parent[1], buf, 1);
    }

    daemon_delete(daemon);
}
END_TEST

Suite *check_suite(void) {
    Suite *s = suite_create("check daemon");

    TCase *tc_core = tcase_create("Daemon");
    tcase_add_test(tc_core, check_daemon_new_and_delete);
    tcase_add_test(tc_core, check_daemon_can_flock_a_file);
    tcase_add_test(tc_core, check_daemon_fails_to_flock_an_already_flocked_file);
    suite_add_tcase(s, tc_core);

    return s;
}

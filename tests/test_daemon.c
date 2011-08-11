#include <assert.h>

#include <fcntl.h>
#include "unit_testing.h"

#include "daemon.h"

void test_daemon_new_and_delete_does_not_leak_memory(void **state) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    daemon = daemon_new(config);
    daemon_delete(daemon);
}

void test_daemon_can_flock_a_file(void **state) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->lock_filename = "test.lock";
    daemon = daemon_new(config);

    assert_int_equal(daemon_get_lock(daemon), SUCCESS);

    daemon_delete(daemon);
}

void test_daemon_fails_to_flock_an_already_flocked_file(void **state) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->lock_filename = "test.lock";
    daemon = daemon_new(config);

    int to_parent[2];
    int to_child[2];
    assert(pipe(to_parent) == 0);
    assert(pipe(to_child) == 0);
    int lock_fd = 0;
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_len = 0,
        .l_pid = 1
    };
    pid_t pid = fork();
    assert(pid >= 0);
    char buf[1];

    if (pid == 0) {
        // parent
        read(to_parent[0], buf, 1);
        assert_int_equal(daemon_get_lock(daemon), FAILURE);
        write(to_child[1], buf, 1);
        read(to_parent[0], buf, 1);
        assert_int_equal(daemon_get_lock(daemon), SUCCESS);
    } else {
        // child
        lock_fd = open(config->lock_filename, O_RDWR | O_CREAT, 0640);
        assert_int_not_equal(lock_fd, -1);
        assert_int_not_equal(fcntl(lock_fd, F_SETLK, &lock), -1);
        write(to_parent[1], buf, 1);
        read(to_child[0], buf, 1);
        lock.l_type = F_UNLCK;
        assert_int_not_equal(fcntl(lock_fd, F_SETLK, &lock), -1);
        write(to_parent[1], buf, 1);
    }

    daemon_delete(daemon);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_daemon_new_and_delete_does_not_leak_memory),
        unit_test(test_daemon_can_flock_a_file),
        unit_test(test_daemon_fails_to_flock_an_already_flocked_file),
    };
    return run_tests(tests);
}

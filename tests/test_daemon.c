#include <assert.h>
#include "unit_testing.h"

#include "daemon.h"

void test_daemon_new_and_delete(void **state) {
    struct daemon *daemon = NULL;
    daemon = daemon_new();
    daemon_delete(daemon);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_daemon_new_and_delete),
    };
    return run_tests(tests);
}

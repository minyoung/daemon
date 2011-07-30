#include <assert.h>
#include "unit_testing.h"

#include "config.h"

void test_config_new_and_delete_does_not_leak_memory(void **state) {
    struct config *config = NULL;
    config = config_new();
    config_delete(config);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_config_new_and_delete_does_not_leak_memory),
    };
    return run_tests(tests);
}


#include <assert.h>
#include "unit_testing.h"

#include "logging.h"

void test_logging(void **state) {
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_logging),
    };
    return run_tests(tests);
}

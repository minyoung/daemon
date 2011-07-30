#include <assert.h>
#include "unit_testing.h"

#include "common.h"

void test_string_copy(void **state) {
    char *src1 = "hello world!";
    char *src2 = "hello back";

    char *dest = string_copy(NULL, src1);
    assert_string_equal(dest, "hello world!");

    assert(string_copy(&dest, src2) == dest);
    assert_string_equal(dest, "hello back");

    assert(string_copy(&dest, src1) == dest);
    assert_string_equal(dest, "hello world!");
    /* free(dest); */
}

void test_copy_if_null(void **state) {
    char *dest = NULL;
    char *src1 = "hello world!";
    char *src2 = "hello back";

    assert(copy_if_null(&dest, src1) == dest);
    assert_string_equal(dest, "hello world!");
    assert(copy_if_null(&dest, src2) == 0);
    assert_string_equal(dest, "hello world!");
    /* free(dest); */
}

void test_free_pointer(void **state) {
    char *ptr = NULL;

    free_pointer(&ptr);
    assert(ptr == NULL);

    ptr = malloc(sizeof(int));
    assert(ptr != NULL);
    free_pointer(&ptr);
    assert(ptr == NULL);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_string_copy),
        unit_test(test_copy_if_null),
        unit_test(test_free_pointer),
    };
    return run_tests(tests);
}

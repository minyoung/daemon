#include <check.h>
#include <stdlib.h>

Suite *check_suite(void);

#define ck_assert_memory_eq(a, b, size) \
        _ck_assert_memory_eq((const char *)a, (const char *)b, size)
void _ck_assert_memory_eq(const char *a, const char *b, int count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        fail_unless(a[i] == b[i], "array check failed (%d) %x != %x", i, a[i], b[i]);
    }
}

int main(int argc, char **argv) {
    int number_failed = 0;
    Suite *s = check_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}

#include "checks.h"

void _ck_assert_memory_eq(const char *a, const char *b, int count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        fail_unless(a[i] == b[i], "array check failed (%d) %x != %x", i, a[i], b[i]);
    }
}

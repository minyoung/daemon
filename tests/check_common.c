#include "common.h"
#include "checks.h"

#include <stdlib.h>

START_TEST (check_string_copy) {
    char *src1 = "hello world!";
    char *src2 = "hello back";

    char *dest = string_copy(NULL, src1);
    ck_assert_str_eq(dest, "hello world!");

    ck_assert(string_copy(&dest, src2) == dest);
    ck_assert_str_eq(dest, "hello back");

    ck_assert(string_copy(&dest, src1) == dest);
    ck_assert_str_eq(dest, "hello world!");
    free(dest);
}
END_TEST

START_TEST (check_copy_if_null) {
    char *dest = NULL;
    char *src1 = "hello world!";
    char *src2 = "hello back";

    ck_assert(copy_if_null(&dest, src1) == dest);
    ck_assert_str_eq(dest, "hello world!");
    ck_assert(copy_if_null(&dest, src2) == 0);
    ck_assert_str_eq(dest, "hello world!");
    free(dest);
}
END_TEST

START_TEST (check_free_pointer) {
    char *ptr = NULL;

    free_pointer(&ptr);
    ck_assert(ptr == NULL);

    ptr = malloc(sizeof(int));
    ck_assert(ptr != NULL);
    free_pointer(&ptr);
    ck_assert(ptr == NULL);
}
END_TEST


Suite *make_common_suite(void) {
    Suite *s = suite_create("check common");

    TCase *tc_core = tcase_create("Common");
    tcase_add_test(tc_core, check_string_copy);
    tcase_add_test(tc_core, check_copy_if_null);
    tcase_add_test(tc_core, check_free_pointer);
    suite_add_tcase(s, tc_core);

    return s;
}

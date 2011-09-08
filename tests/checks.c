#include <stdlib.h>
#include <stdio.h>

#include "checks.h"

struct test_suite {
    char *name;
    int len;
    Suite *(*make_suite)(void);
};

static int global_suites_count = 0;
static struct test_suite *global_suites;

struct test_suite make_test_suite(char *name, Suite *(*make_suite)(void)) {
    global_suites_count += 1;
    struct test_suite suite;
    suite.name = name;
    suite.len = strlen(name);
    suite.make_suite = make_suite;
    return suite;
}

Suite *find_suite(char *name) {
    unsigned int len = strlen(name);
    int i = 0;
    for (i = 0; i < global_suites_count; i++) {
        if (len != global_suites[i].len) {
            continue;
        }
        if (strncasecmp(name, global_suites[i].name, len) == 0) {
            return global_suites[i].make_suite();
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("%s SUITE1 SUITE2 SUITE3...\n", argv[0]);
        printf("Using \"all\" as the suite name runs all test suites\n");
        return 1;
    }

    struct test_suite suites[] = {
        make_test_suite("common", make_common_suite),
        make_test_suite("config", make_config_suite),
        make_test_suite("daemon", make_daemon_suite),
        make_test_suite("logging", make_logging_suite),
        make_test_suite("network", make_network_suite),
        make_test_suite("packet", make_packet_suite),
        make_test_suite("storage", make_storage_suite),
    };
    global_suites = suites;


    int number_failed = 0;

    Suite *s = NULL;
    SRunner *sr = srunner_create(NULL);

    int i = 0;
    int total = 0;
    if (strncmp(argv[1], "all", strlen(argv[1])) == 0) {
        for (i = 0; i < global_suites_count; i++) {
            srunner_add_suite(sr, suites[i].make_suite());
            total += 1;
        }
    } else {
        for (i = 1; i < argc; i++) {
            s = find_suite(argv[i]);
            if (s != NULL) {
                srunner_add_suite(sr, s);
                total += 1;
            } else {
                printf("Could not find matching test suite: %s\n", argv[i]);
            }
        }
    }

    if (total == 0) {
        srunner_free(sr);
        printf("Not running any test suites\n");
        return 1;
    }

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}

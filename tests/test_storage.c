#include <assert.h>
#include "unit_testing.h"

#include "storage.h"

struct daemon *stub_daemon() {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    daemon = daemon_new(config);

    return daemon;
}

void test_hash_value(void **state) {
    unsigned char hash[] = {
        0x8f, 0xfd, 0x54, 0x5d, 0x84, 0xfd, 0x53, 0x03, 0xd8, 0x67,
        0x58, 0x63, 0x2e, 0xf5, 0xe9, 0x87, 0x2a, 0x3d, 0x56, 0xbb
    };

    unsigned char *result = NULL;
    struct stats_packet *packet = NULL;
    struct daemon *daemon = NULL;

    daemon = stub_daemon();
    packet = malloc(sizeof(*packet));
    string_copy(&packet->service, "service");
    string_copy(&packet->metric, "metric");

    result = storage_get_stats_hash(daemon, packet);

    assert_memory_equal(hash, result, sizeof(hash));
    free(packet);
    daemon_delete(daemon);
}

void test_store_stats_data(void **state) {
    struct daemon *daemon = stub_daemon();
    struct stats_packet *packet = malloc(sizeof(*packet));
    char hash[] = "12345678911234567892";

    assert(storage_create_paths(daemon, hash) == SUCCESS);
    assert(storage_store_stats_data(daemon, hash, packet) == SUCCESS);

    assert_int_equal(unlink("12/34/5678911234567892"), 0);
    assert_int_equal(rmdir("12/34"), 0);
    assert_int_equal(rmdir("12"), 0);

    free(packet);
    daemon_delete(daemon);
}

void test_storage_data_filename(void **state) {
    struct daemon *daemon = stub_daemon();

    assert_string_equal(storage_format_data_filename(daemon, "12345678911234567892"), "12/34/5678911234567892");

    daemon_delete(daemon);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_hash_value),
        unit_test(test_store_stats_data),
        unit_test(test_storage_data_filename),
    };
    return run_tests(tests);
}

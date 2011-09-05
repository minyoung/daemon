#include "storage.h"
#include "checkhelper.c"

struct daemon *stub_daemon() {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    daemon = daemon_new(config);

    return daemon;
}

struct stats_packet *stub_stats_packet() {
    struct stats_packet *packet = malloc(sizeof(*packet));
    packet->type = NULL;
    packet->service = NULL;
    packet->metric = NULL;
    packet->hostname = NULL;
    packet->type = string_copy(&packet->type, "5m.p50");
    packet->service = string_copy(&packet->service, "service");
    packet->metric = string_copy(&packet->metric, "metric");
    packet->hostname = string_copy(&packet->hostname, "hostname");

    packet->tag_count = 2;
    packet->tags = malloc(2 * sizeof(char *));
    packet->tags[0] = NULL;
    packet->tags[1] = NULL;
    packet->tags[0] = string_copy(&packet->tags[0], "tag1");
    packet->tags[1] = string_copy(&packet->tags[1], "tag2");
    packet->timestamp = 72623859790382856ll;
    packet->value = 72623859790382856ll;

    return packet;
}

START_TEST (check_hash_value) {
    unsigned char hash[] = {
        0x8f, 0xfd, 0x54, 0x5d, 0x84, 0xfd, 0x53, 0x03, 0xd8, 0x67,
        0x58, 0x63, 0x2e, 0xf5, 0xe9, 0x87, 0x2a, 0x3d, 0x56, 0xbb
    };

    unsigned char *result = NULL;
    struct stats_packet *packet = NULL;
    struct daemon *daemon = NULL;

    daemon = stub_daemon();
    packet = stub_stats_packet();
    string_copy(&packet->service, "service");
    string_copy(&packet->metric, "metric");

    result = storage_get_stats_hash(daemon, packet);

    ck_assert_memory_eq(hash, result, sizeof(hash));
    free(result);
    stats_packet_delete(packet);
    daemon_delete(daemon);
}
END_TEST

START_TEST (check_store_stats_data) {
    struct daemon *daemon = stub_daemon();
    struct stats_packet *packet = stub_stats_packet();
    char hash[] = "\x11\x22\x33\x44\x55\x66\x77\x88\x99\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\x22";
    ck_assert(daemon != NULL);
    ck_assert(packet != NULL);

    ck_assert(storage_store_stats_data(daemon, hash, packet) == SUCCESS);

    ck_assert(access("11/22/334455667788990011223344556677889922", F_OK) == 0);
    ck_assert(unlink("11/22/334455667788990011223344556677889922") == 0);
    ck_assert(rmdir("11/22") == 0);
    ck_assert(rmdir("11") == 0);

    stats_packet_delete(packet);
    daemon_delete(daemon);
}
END_TEST

START_TEST (check_storage_data_filename) {
    struct daemon *daemon = stub_daemon();

    char *filename = storage_format_data_filename(daemon, "\x11\x22\x33\x44\x55\x66\x77\x88\x99\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\x22");
    ck_assert_str_eq(filename, "11/22/334455667788990011223344556677889922");

    free(filename);
    daemon_delete(daemon);
}
END_TEST

START_TEST (check_storage_write_header) {
    struct daemon *self = stub_daemon();
    struct stats_packet *packet = stub_stats_packet();
    char *header = "S0\n5m.p50\nservice\nmetric\nhostname\ntag1\ntag2\n\n\x8\x7\x6\x5\x4\x3\x2\x1";
    char written[53];

    int fd[2];
    ck_assert(pipe(fd) == 0);
    FILE *output_stream = fdopen(fd[1], "w");

    ck_assert(storage_write_header(self, output_stream, packet) == SUCCESS);

    fclose(output_stream);
    ck_assert(read(fd[0], written, sizeof(written)) > 10);
    ck_assert_memory_eq(written, header, sizeof(written));

    daemon_delete(self);
    stats_packet_delete(packet);
}
END_TEST

Suite *check_suite(void) {
    Suite *s = suite_create("check storage");

    TCase *tc_core = tcase_create("Storage");
    /* tcase_add_test(tc_core, check_hash_value); */
    tcase_add_test(tc_core, check_store_stats_data);
    /* tcase_add_test(tc_core, check_storage_data_filename); */
    /* tcase_add_test(tc_core, check_storage_write_header); */
    suite_add_tcase(s, tc_core);

    return s;
}


#include "packet.h"
#include "checks.h"

START_TEST (check_packet_create_and_delete) {
    struct packet *self = NULL;
    self = packet_create();
    packet_delete(self);
}
END_TEST

START_TEST (check_stats_packet_create_extracts_the_correct_values) {
    struct packet *packet = NULL;
    char d[] = {
        /* timestamp */
        1, 0, 0, 0, 0, 0, 0, 0,
        /* value */
        2, 0, 0, 0, 0, 0, 0, 0,
        /* service */
        8, 's', 'e', 'r', 'v', 'i', 'c', 'e', 0,
        /* metric */
        7, 'm', 'e', 't', 'r', 'i', 'c', 0,
        /* hostname */
        9, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 0,
    };
    packet = packet_create();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct stats_packet *self = NULL;
    self = stats_packet_create(packet);
    ck_assert(1 == self->timestamp);
    ck_assert(2 == self->value);
    ck_assert_str_eq(self->service, "service");
    ck_assert_str_eq(self->metric, "metric");
    ck_assert_str_eq(self->hostname, "hostname");
    ck_assert_int_eq(self->tag_count, 0);

    stats_packet_delete(self);
    packet_delete(packet);
}
END_TEST

START_TEST (check_stats_packet_create_handles_null_padded_strings) {
    struct packet *packet = NULL;
    char d[] = {
        /* timestamp */
        1, 0, 0, 0, 0, 0, 0, 0,
        /* value */
        2, 0, 0, 0, 0, 0, 0, 0,
        /* service */
        11, 's', 'e', 'r', 'v', 'i', 'c', 'e', 0, 0, 0, 0,
        /* metric */
        7, 'm', 'e', 't', 'r', 'i', 'c', 0,
        /* hostname */
        11, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 0, 0, 0,
        /* tags */
        7, 't', 'a', 'g', '1', 0, 0, 0,
        7, 't', 'a', 'g', '2', 0, 0, 0,
    };
    packet = packet_create();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct stats_packet *self = NULL;
    self = stats_packet_create(packet);
    ck_assert(1 == self->timestamp);
    ck_assert(2 == self->value);
    ck_assert_str_eq(self->service, "service");
    ck_assert_str_eq(self->metric, "metric");
    ck_assert_str_eq(self->hostname, "hostname");
    ck_assert_int_eq(self->tag_count, 2);
    ck_assert_str_eq(self->tags[0], "tag1");
    ck_assert_str_eq(self->tags[1], "tag2");

    stats_packet_delete(self);
    packet_delete(packet);
}
END_TEST

START_TEST (check_log_packet_create_extracts_the_correct_values) {
    struct packet *packet = NULL;
    char d[] = {
        /* timestamp */
        1, 0, 0, 0, 0, 0, 0, 0,
        /* service */
        8, 's', 'e', 'r', 'v', 'i', 'c', 'e', 0,
        /* log_line */
        9, 'l', 'o', 'g', ' ', 'l', 'i', 'n', 'e', 0,
        /* hostname */
        9, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 0,
        /* tags */
        7, 't', 'a', 'g', '1', 0, 0, 0,
    };
    packet = packet_create();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct log_packet *self = NULL;
    self = log_packet_create(packet);
    ck_assert(1 == self->timestamp);
    ck_assert_str_eq(self->service, "service");
    ck_assert_str_eq(self->log_line, "log line");
    ck_assert_str_eq(self->hostname, "hostname");
    ck_assert_int_eq(self->tag_count, 1);

    log_packet_delete(self);
    packet_delete(packet);
}
END_TEST


Suite *make_packet_suite(void) {
    Suite *s = suite_create("check packet");

    TCase *tc_core = tcase_create("Packet");
    tcase_add_test(tc_core, check_packet_create_and_delete);
    tcase_add_test(tc_core, check_stats_packet_create_extracts_the_correct_values);
    tcase_add_test(tc_core, check_stats_packet_create_handles_null_padded_strings);
    tcase_add_test(tc_core, check_log_packet_create_extracts_the_correct_values);
    suite_add_tcase(s, tc_core);

    return s;
}

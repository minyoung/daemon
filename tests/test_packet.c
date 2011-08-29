#include <assert.h>
#include "unit_testing.h"

#include "packet.h"

#include <string.h>

#include <stdio.h>

void test_packet_new_and_delete_does_not_leak_memory(void **state) {
    struct packet *self = NULL;
    self = packet_new();
    packet_delete(self);
}

void test_stats_packet_new_extracts_the_correct_values(void **state) {
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
    packet = packet_new();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct stats_packet *self = NULL;
    self = stats_packet_new(packet);
    assert(1 == self->timestamp);
    assert(2 == self->value);
    assert_string_equal(self->service, "service");
    assert_string_equal(self->metric, "metric");
    assert_string_equal(self->hostname, "hostname");
    assert_int_equal(self->tag_count, 0);

    stats_packet_delete(self);
}

void test_stats_packet_new_handles_null_padded_strings(void **state) {
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
    packet = packet_new();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct stats_packet *self = NULL;
    self = stats_packet_new(packet);
    assert(1 == self->timestamp);
    assert(2 == self->value);
    assert_string_equal(self->service, "service");
    assert_string_equal(self->metric, "metric");
    assert_string_equal(self->hostname, "hostname");
    assert_int_equal(self->tag_count, 2);
    assert_string_equal(self->tags[0], "tag1");
    assert_string_equal(self->tags[1], "tag2");

    stats_packet_delete(self);
}

void test_log_packet_new_extracts_the_correct_values(void **state) {
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
    packet = packet_new();
    memcpy(packet->message, d, sizeof(d));
    packet->len = sizeof(d);

    struct log_packet *self = NULL;
    self = log_packet_new(packet);
    assert(1 == self->timestamp);
    assert_string_equal(self->service, "service");
    assert_string_equal(self->log_line, "log line");
    assert_string_equal(self->hostname, "hostname");
    assert_int_equal(self->tag_count, 1);

    log_packet_delete(self);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_packet_new_and_delete_does_not_leak_memory),
        unit_test(test_stats_packet_new_extracts_the_correct_values),
        unit_test(test_stats_packet_new_handles_null_padded_strings),
        unit_test(test_log_packet_new_extracts_the_correct_values),
    };
    return run_tests(tests);
}

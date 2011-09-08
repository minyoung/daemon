#ifndef CHECKS_H
#define CHECKS_H

#include <check.h>

#define ck_assert_memory_eq(a, b, size) \
        _ck_assert_memory_eq((const char *)a, (const char *)b, size)
void _ck_assert_memory_eq(const char *a, const char *b, int count);

Suite *make_common_suite(void);
Suite *make_config_suite(void);
Suite *make_daemon_suite(void);
Suite *make_logging_suite(void);
Suite *make_network_suite(void);
Suite *make_packet_suite(void);
Suite *make_storage_suite(void);

#endif

#include "network.h"
#include "checkhelper.c"

#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
/* #include <sys/socket.h> */
/* #include <sys/types.h> */

#include "common.h"
#include "daemon.h"
#include "packet.h"

START_TEST (check_network_can_open_a_socket) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_create();
    daemon = daemon_create(config);

    ck_assert(network_open_socket(daemon, "7357") > 0);

    daemon_delete(daemon);
}
END_TEST

START_TEST (check_network_can_receive_and_send_packets) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_create();
    config->client_port = string_copy(&config->client_port, "7357");
    daemon = daemon_create(config);

    int fd[2];
    char buf[1];
    ck_assert(pipe(fd) == 0);

    pid_t pid = fork();
    ck_assert(pid >= 0);

    if (pid == 0) {
        // parent
        daemon->network_sockets[DAEMON_CLIENT] = network_open_socket(daemon, "7357");
        ck_assert(daemon->network_sockets[DAEMON_CLIENT] != 0);
        write(fd[1], buf, 1);
        network_handle_client_socket(daemon);
    } else {
        // child
        read(fd[0], buf, 1);
        struct sockaddr_in server_addr;
        struct addrinfo hints;
        struct addrinfo *result;
        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        ck_assert(getaddrinfo("localhost", "7357", &hints, &result) == 0);

        struct addrinfo *rp;
        int client_socket;
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            client_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (client_socket == -1)
                continue;

            if (connect(client_socket, rp->ai_addr, rp->ai_addrlen) != -1)
                break;

            close(client_socket);
        }
        ck_assert(rp != NULL);
        freeaddrinfo(result);

        struct packet pkt;
        pkt.type = 'E';
        pkt.version = 1;
        pkt.len = 13;
        strncpy(pkt.message, "Hello world!\0", pkt.len);

        int pkt_size = pkt.len + 4;
        int read_size = 0;
        ck_assert(write(client_socket, &pkt, pkt_size) == pkt_size);
        pkt.type = 0;
        pkt.version = 0;
        pkt.len = 0;
        pkt.message[0] = 0;
        ck_assert(read(client_socket, &pkt, pkt_size) == pkt_size);

        ck_assert_int_eq(pkt.type, 'E');
        ck_assert_int_eq(pkt.version, 1);
        ck_assert_int_eq(pkt.len, 13);
        ck_assert_str_eq(pkt.message, "Hello world!\0");

        pkt.type = 'Q';
        pkt.len = 0;
        pkt.message[0] = 0;
        pkt_size = pkt.len + 4;
        ck_assert(write(client_socket, &pkt, pkt_size) == pkt_size);
    }

    daemon_delete(daemon);
}
END_TEST

Suite *check_suite(void) {
    Suite *s = suite_create("check network");

    TCase *tc_core = tcase_create("Network");
    tcase_add_test(tc_core, check_network_can_open_a_socket);
    tcase_add_test(tc_core, check_network_can_receive_and_send_packets);
    suite_add_tcase(s, tc_core);

    return s;
}

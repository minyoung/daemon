#include <assert.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
/* #include <sys/socket.h> */
/* #include <sys/types.h> */

#include "unit_testing.h"

#include "common.h"
#include "daemon.h"
#include "network.h"
#include "packet.h"

void test_network_can_open_a_socket(void **state) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->port = "7357";
    daemon = daemon_new(config);

    assert(network_open_socket(daemon) == SUCCESS);

    daemon_delete(daemon);
}

void test_network_can_receive_and_send_packets(void **state) {
    struct daemon *daemon = NULL;
    struct config *config = NULL;
    config = config_new();
    config->port = "7357";
    daemon = daemon_new(config);

    int fd[2];
    char buf[1];
    assert(pipe(fd) == 0);

    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        // parent
        assert(network_open_socket(daemon) == SUCCESS);
        write(fd[1], buf, 1);
        network_handle_socket(daemon);
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
        assert_int_equal(getaddrinfo("localhost", "7357", &hints, &result), 0);

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
        assert(rp != NULL);
        freeaddrinfo(result);

        struct packet pkt;
        pkt.type = 'Q';
        pkt.version = 1;
        pkt.len = 13;
        strncpy(pkt.message, "Hello world!\0", pkt.len);

        int pkt_size = 17;
        int read_size = 0;
        assert_int_equal(write(client_socket, &pkt, pkt_size), pkt_size);
        pkt.type = 0;
        pkt.version = 0;
        pkt.len = 0;
        pkt.message[0] = 0;
        assert_int_equal(read(client_socket, &pkt, pkt_size), pkt_size);

        assert_int_equal(pkt.type, 'Q');
        assert_int_equal(pkt.version, 1);
        assert_int_equal(pkt.len, 13);
        assert_string_equal(pkt.message, "Hello world!\0");
    }

    daemon_delete(daemon);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_network_can_open_a_socket),
        unit_test(test_network_can_receive_and_send_packets),
    };
    return run_tests(tests);
}


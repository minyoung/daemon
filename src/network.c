#include "network.h"

#include <netinet/in.h>
#include <strings.h>
#include <netdb.h>
/* #include <sys/socket.h> */
/* #include <sys/types.h> */

#include "logging.h"
#include "packet.h"

void network_close_socket(struct daemon *self) {
    if (self->server_socket != 0) {
        close(self->server_socket);
        self->server_socket = 0;
    }
}

status_t network_open_socket(struct daemon *self) {
    struct addrinfo hints;
    struct addrinfo *result;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int s;
    if ((s = getaddrinfo(NULL, self->config->port, &hints, &result)) != 0) {
        daemon_err(self, LOG_ERR, "getaddrinfo: %s", gai_strerror(s));
        return FAILURE;
    }

    struct addrinfo *rp;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        self->server_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (self->server_socket == -1)
            continue;

        if (bind(self->server_socket, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(self->server_socket);
    }

    if (rp == NULL) {
        daemon_err(self, LOG_ERR, "Could not bind to socket [%m]");
        return FAILURE;
    }
    freeaddrinfo(result);

    daemon_log(self, LOG_INFO, "Opened socket: %s [%m]", self->config->port);
    return SUCCESS;
}

void network_handle_packet(struct daemon *self, struct packet *packet, struct sockaddr *client_addr, char *host, char *service) {
    int client_len = sizeof(*client_addr);
    int n;
    switch (packet->type) {
    case 'E':
        n = sendto(self->server_socket, packet, sizeof(*packet), 0, client_addr, client_len);
        if (n < 0) {
            daemon_err(self, LOG_ERR, "Error writing to socket [%m]");
        } else {
            daemon_log(self, LOG_DEBUG, "Wrote %ld bytes to %s:%s", n, host, service);
        }
        break;
    case 'Q':
        self->running = 0;
        break;
    }
}

void network_handle_socket(struct daemon *self) {
    struct sockaddr_in client_addr;
    struct sockaddr* client_sock = (struct sockaddr *)&client_addr;
    int client_len = sizeof(client_addr);
    int n, s;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct packet buffer;
    while (self->running) {
        n = recvfrom(self->server_socket, &buffer, sizeof(buffer), MSG_WAITALL, client_sock, &client_len);
        if (n < 0) {
            daemon_err(self, LOG_ERR, "Error reading from socket [%m]");
            continue;
        }

        s = getnameinfo(client_sock, client_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0) {
            daemon_log(self, LOG_DEBUG, "Received %ld bytes from %s:%s", n, host, service);
        } else {
            daemon_log(self, LOG_WARNING, "getnameinfo: %s", gai_strerror(s));
        }

        network_handle_packet(self, &buffer, client_sock, host, service);
    }
}


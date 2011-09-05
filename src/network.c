#include "network.h"

#include <netinet/in.h>
#include <strings.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/time.h>

#include "logging.h"
#include "packet.h"
#include "storage.h"

void *network_handle_client_socket(void *args);
void *network_handle_control_socket(void *args);

void network_close_sockets(struct daemon *self) {
    int i;
    int fd;
    for (i = 0; i < 2; i++) {
        fd = self->network_sockets[i];
        if (fd != 0) {
            close(fd);
            self->network_sockets[i] = 0;
        }
    }
}

status_t network_open_sockets(struct daemon *self) {
    self->network_sockets[DAEMON_CLIENT] = network_open_socket(self, self->config->client_port);
    if (self->network_sockets[DAEMON_CLIENT] == 0) {
        return FAILURE;
    }

    self->network_sockets[DAEMON_CONTROL] = network_open_socket(self, self->config->control_port);
    if (self->network_sockets[DAEMON_CONTROL] == 0) {
        return FAILURE;
    }

    return SUCCESS;
}

int network_open_socket(struct daemon *self, char *port) {
    int socket_fd = 0;
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
    if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0) {
        daemon_err(self, LOG_ERR, "getaddrinfo: %s", gai_strerror(s));
        return socket_fd;
    }

    struct addrinfo *rp;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)
            continue;

        if (bind(socket_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(socket_fd);
        socket_fd = 0;
    }

    if (rp == NULL) {
        daemon_err(self, LOG_ERR, "Could not bind to socket [%m]");
        socket_fd = 0;
        return socket_fd;
    }
    freeaddrinfo(result);

    daemon_log(self, LOG_INFO, "Opened socket: %s [%m]", port);
    return socket_fd;
}

status_t network_start_threads(struct daemon *self) {
    if (pthread_create(&(self->network_threads[DAEMON_CLIENT]), NULL, network_handle_client_socket, (void *)self) != 0) {
        return FAILURE;
    }

    if (pthread_create(&(self->network_threads[DAEMON_CONTROL]), NULL, network_handle_control_socket, (void *)self) != 0) {
        self->running = 0;
        return FAILURE;
    }

    return SUCCESS;
}

void network_handle_client_packet(struct daemon *self, struct packet *packet, int client_socket, struct sockaddr *client_addr, char *host, char *service) {
    int client_len = sizeof(*client_addr);
    int n;
    struct stats_packet *pkt;
    char *tags;

    switch (packet->type) {
    case 'S':
        pkt = stats_packet_create(packet);
        if (storage_store_stats(self, pkt) != SUCCESS) {
        } else {
        }

        tags = malloc(pkt->tag_count * 28 * sizeof(char));
        tags[0] = 0;
        if (pkt->tag_count > 0) {
            sprintf(tags, "'%s'", pkt->tags[0]);
            for (n = 1; n < pkt->tag_count; n++) {
                sprintf(tags, "%s, '%s'", tags, pkt->tags[n]);
            }
        }
        daemon_log(self, LOG_DEBUG, "%s: %s :: %s = %lld (%lld)", pkt->hostname, pkt->service, pkt->metric, pkt->value, pkt->timestamp);
        daemon_log(self, LOG_DEBUG, "Tags (%d): %s", pkt->tag_count, tags);
        stats_packet_delete(pkt);
        /* break; */

    case 'L':
    case 'E':
        n = sendto(client_socket, packet, packet->len + PACKET_HEADER_SIZE, 0, client_addr, client_len);
        if (n < 0) {
            daemon_err(self, LOG_ERR, "Error writing to socket [%m]");
        } else {
            daemon_log(self, LOG_DEBUG, "Wrote %ld bytes to %s:%s", n, host, service);
        }
        break;
    }
}

void *network_handle_client_socket(void *args) {
    struct daemon *self = (struct daemon *)args;

    struct sockaddr_in client_addr;
    struct sockaddr* client_sock = (struct sockaddr *)&client_addr;
    int client_len = sizeof(client_addr);

    int client_socket = self->network_sockets[DAEMON_CLIENT];

    int n, s;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct packet buffer;

    fd_set fds;
    struct timeval timeout;

    daemon_log(self, LOG_DEBUG, "Client network thread started [%m]");
    while (self->running) {
        FD_ZERO(&fds);
        FD_SET(client_socket, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        if (select(client_socket+1, &fds, NULL, NULL, &timeout) == 0) {
            /* daemon_log(self, LOG_DEBUG, "No client requests [%m]"); */
            continue;
        }

        n = recvfrom(client_socket, &buffer, sizeof(buffer), MSG_WAITALL, client_sock, &client_len);
        if (n < 0) {
            daemon_err(self, LOG_ERR, "Error reading from client socket [%m]");
            continue;
        }

        s = getnameinfo(client_sock, client_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0) {
            daemon_log(self, LOG_DEBUG, "Received %ld bytes from client %s:%s", n, host, service);
        } else {
            daemon_log(self, LOG_WARNING, "getnameinfo: %s", gai_strerror(s));
        }

        network_handle_client_packet(self, &buffer, client_socket, client_sock, host, service);
    }
    daemon_log(self, LOG_DEBUG, "Client network thread ending [%m]");
}

void network_handle_control_packet(struct daemon *self, struct packet *packet, int control_socket, struct sockaddr *client_addr, char *host, char *service) {
    int client_len = sizeof(*client_addr);
    int n;
    switch (packet->type) {
    case 'E':
        n = sendto(control_socket, packet, packet->len + PACKET_HEADER_SIZE, 0, client_addr, client_len);
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

void *network_handle_control_socket(void *args) {
    struct daemon *self = (struct daemon *)args;
    struct sockaddr_in client_addr;
    struct sockaddr* client_sock = (struct sockaddr *)&client_addr;
    int client_len = sizeof(client_addr);

    int control_socket = self->network_sockets[DAEMON_CONTROL];

    int n, s;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    struct packet buffer;

    fd_set fds;
    struct timeval timeout;

    daemon_log(self, LOG_DEBUG, "Control network thread started [%m]");
    while(self->running) {
        FD_ZERO(&fds);
        FD_SET(control_socket, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        if (select(control_socket+1, &fds, NULL, NULL, &timeout) == 0) {
            /* daemon_log(self, LOG_DEBUG, "No control requests [%m]"); */
            continue;
        }

        n = recvfrom(control_socket, &buffer, sizeof(buffer), MSG_WAITALL, client_sock, &client_len);
        if (n < 0) {
            daemon_err(self, LOG_ERR, "Error reading from control socket [%m]");
            continue;
        }

        s = getnameinfo(client_sock, client_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0) {
            daemon_log(self, LOG_DEBUG, "Received %ld bytes from control %s:%s", n, host, service);
        } else {
            daemon_log(self, LOG_WARNING, "getnameinfo: %s", gai_strerror(s));
        }

        network_handle_control_packet(self, &buffer, control_socket, client_sock, host, service);
    }

    daemon_log(self, LOG_DEBUG, "Control network thread ending [%m]");
}


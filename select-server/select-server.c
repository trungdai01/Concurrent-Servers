#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define FD_SETSIZE 1024
#include "utils.h"

#define MAXFDs 1000

typedef enum {
    INITIAL_ACK,
    WAIT_FOR_MSG,
    IN_MSG,
} ProcessingState;

#define SENDBUF_SIZE 1024

typedef struct {
    ProcessingState state;
    uint8_t sendbuf[SENDBUF_SIZE];
    int sendbuf_end;
    int sendptr;
} peer_state_t;

peer_state_t global_state[MAXFDs];

typedef struct {
    bool become_readable;
    bool become_writable;
} fd_status_t;

const fd_status_t fd_status_R = {
    .become_readable = true,
    .become_writable = false,
};
const fd_status_t fd_status_W = {
    .become_readable = false,
    .become_writable = true,
};
const fd_status_t fd_status_RW = {
    .become_readable = true,
    .become_writable = true,
};
const fd_status_t fd_status_NORW = {
    .become_readable = false,
    .become_writable = false,
};

fd_status_t on_peer_connected(int client_sockfd, const struct sockaddr_in* peer_addr, socklen_t peer_addr_len) {
    assert(client_sockfd < MAXFDs);
    report_peer_connected(peer_addr, peer_addr_len);

    peer_state_t* peer_handler = &global_state[client_sockfd];
    peer_handler->state = INITIAL_ACK;
    peer_handler->sendbuf[0] = (uint8_t)'*';
    peer_handler->sendptr = 0;
    peer_handler->sendbuf_end = 1;

    return fd_status_W;
}

fd_status_t on_peer_received(int client_sockfd) {
    assert(client_sockfd < MAXFDs);
    peer_state_t* peer_handler = &global_state[client_sockfd];

    if (peer_handler->state == INITIAL_ACK || peer_handler->sendptr < peer_handler->sendbuf_end) {
        // Until the initial ACK has been sent to the peer, there's nothing we
        // want to receive. Also, wait until all data staged for sending is sent to
        // receive more data.
        return fd_status_W;
    }

    uint8_t buf[1024];
    int bytesRecv = recv(client_sockfd, buf, sizeof buf, 0);
    if (bytesRecv == 0) {
        printf("%d is disconnected\n", client_sockfd);
        return fd_status_NORW;
    } else if (bytesRecv < 0) {
        if (bytesRecv == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
            printf("%d is not ready to receive\n", client_sockfd);
            return fd_status_R;
        } else {
            perror_die("recv");
        }
    }

    bool ready_to_send_back = false;
    for (int i = 0; i < bytesRecv; ++i) {
        switch (peer_handler->state) {
            case WAIT_FOR_MSG:
                if (buf[i] == '^') peer_handler->state = IN_MSG;
                break;
            case IN_MSG:
                if (buf[i] == '$') {
                    peer_handler->state = WAIT_FOR_MSG;
                } else {
                    assert(peer_handler->sendbuf_end < SENDBUF_SIZE);
                    peer_handler->sendbuf[peer_handler->sendbuf_end++] = buf[i] + 1;
                    ready_to_send_back = true;
                }
                break;
        }
    }

    return (fd_status_t){
        .become_readable = !ready_to_send_back,
        .become_writable = ready_to_send_back,
    };
}

fd_status_t on_peer_sent(int client_sockfd) {
    assert(client_sockfd < MAXFDs);
    peer_state_t* peer_state = &global_state[client_sockfd];

    if (peer_state->sendptr >= peer_state->sendbuf_end) {
        return fd_status_RW;
    }
    int msg_len = peer_state->sendbuf_end - peer_state->sendptr;
    int bytes_sent = send(client_sockfd, &peer_state->sendbuf[peer_state->sendptr], msg_len, 0);
    if (bytes_sent == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return fd_status_W;
        } else {
            perror_die("send");
        }
    }
    if (bytes_sent < msg_len) {
        printf("server is sending message to %d", client_sockfd);
        peer_state->sendptr += bytes_sent;
        return fd_status_W;
    } else {
        printf("server sent messages successfully\n");
        peer_state->sendptr = 0;
        peer_state->sendbuf_end = 0;

        // Special-case state transition in if we were in INITIAL_ACK until now.
        if (peer_state->state == INITIAL_ACK) peer_state->state = WAIT_FOR_MSG;

        return fd_status_R;
    }
}

int main(int argc, char** argv) {
    if (initializeWinsock() != 0) {
        return 1;
    }
    setvbuf(stdout, NULL, _IONBF, 0);

    int portnum = 9090;
    if (argc >= 2) {
        portnum = atoi(argv[1]);
    }
    printf("Serving on port %d\n", portnum);

    int server_sockfd = listen_inet_socket(portnum);
    printf("server sockfd: %d\n", server_sockfd);

    // The select() manpage warns that select() can return a read notification
    // for a socket that isn't actually readable. Thus using blocking I/O isn't
    // safe.
    make_socket_non_blocking(server_sockfd);

    if (server_sockfd >= FD_SETSIZE) {
        die("server socket fd (%d) >= FD_SETSIZE (%d)", server_sockfd, FD_SETSIZE);
    }

    // The "master" sets are owned by the loop, tracking which FDs we want to
    // monitor for receiving and which FDs we want to monitor for sending.
    fd_set readable_fd_monitor_set;
    FD_ZERO(&readable_fd_monitor_set);
    fd_set writable_fd_monitor_set;
    FD_ZERO(&writable_fd_monitor_set);

    // The server socket is always monitored for recv, to detect when new
    // peer connections are incoming.
    FD_SET(server_sockfd, &readable_fd_monitor_set);

    // For more efficiency, fdset_max tracks the maximal FD seen so far; this
    // makes it unnecessary for select to iterate all the way to FD_SETSIZE on
    // every call.
    int fdset_max = server_sockfd;
    int loop_num = 0;

    while (1) {
        fd_set read_fd_set = readable_fd_monitor_set;
        fd_set write_fd_set = writable_fd_monitor_set;

        int num_ready = select(fdset_max + 1, &read_fd_set, &write_fd_set, NULL, NULL);
        if (num_ready == SOCKET_ERROR) {
            perror_die("[MAIN-LOOP] select error");
        }
        printf("Loop: %d, num_ready: %d\n", loop_num++, num_ready);

        // num_ready tells us the total number of ready events; if one socket is both
        // readable and writable it will be 2. Therefore, it's decremented when
        // either a readable or a writable socket is encountered.
        for (int fd = 0; fd <= fdset_max && num_ready > 0; fd++) {
            // Check if this fd became readable.
            if (FD_ISSET(fd, &read_fd_set)) {
                num_ready--;

                printf("[MAIN-LOOP] reading message from %d\n", fd);
                if (fd == server_sockfd) {
                    struct sockaddr_in peer_addr;
                    socklen_t peer_addr_len = sizeof(peer_addr);
                    int client_sockfd = accept(server_sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
                    if (client_sockfd == SOCKET_ERROR) {
                        if (WSAGetLastError() == WSAEWOULDBLOCK) {
                            printf("accept returned EAGAIN or EWOULDBLOCK\n");
                        } else {
                            perror_die("accept");
                        }
                    } else {
                        printf("Established client sockfd: %d\n", client_sockfd);
                        make_socket_non_blocking(client_sockfd);
                        if (client_sockfd > fdset_max) {
                            if (client_sockfd >= FD_SETSIZE) {
                                die("socket fd (%d) >= FD_SETSIZE (%d)", client_sockfd, FD_SETSIZE);
                            }
                            fdset_max = client_sockfd;
                        }

                        fd_status_t client_status = on_peer_connected(client_sockfd, &peer_addr, peer_addr_len);
                        if (client_status.become_readable) {
                            FD_SET(client_sockfd, &readable_fd_monitor_set);
                        } else {
                            FD_CLR(client_sockfd, &readable_fd_monitor_set);
                        }
                        if (client_status.become_writable) {
                            FD_SET(client_sockfd, &writable_fd_monitor_set);
                        } else {
                            FD_CLR(client_sockfd, &writable_fd_monitor_set);
                        }
                    }
                } else {
                    printf("%d sent a new message to server\n", fd);
                    fd_status_t client_status = on_peer_received(fd);
                    if (client_status.become_readable) {
                        FD_SET(fd, &readable_fd_monitor_set);
                    } else {
                        FD_CLR(fd, &readable_fd_monitor_set);
                    }
                    if (client_status.become_writable) {
                        FD_SET(fd, &writable_fd_monitor_set);
                    } else {
                        FD_CLR(fd, &writable_fd_monitor_set);
                    }
                    if (!client_status.become_readable && !client_status.become_writable) {
                        printf("socket %d closing\n", fd);
                        closesocket(fd);
                    }
                }
            }

            // Check if this fd became writable.
            if (FD_ISSET(fd, &write_fd_set)) {
                num_ready--;
                printf("server want to send back a message to %d\n", fd);
                fd_status_t client_status = on_peer_sent(fd);
                if (client_status.become_readable) {
                    FD_SET(fd, &readable_fd_monitor_set);
                } else {
                    FD_CLR(fd, &readable_fd_monitor_set);
                }
                if (client_status.become_writable) {
                    FD_SET(fd, &writable_fd_monitor_set);
                } else {
                    FD_CLR(fd, &writable_fd_monitor_set);
                }
                if (!client_status.become_readable && !client_status.become_writable) {
                    printf("socket %d closing\n", fd);
                    closesocket(fd);
                }
            }
        }
    }
    cleanupWinsock();
    return 0;
}
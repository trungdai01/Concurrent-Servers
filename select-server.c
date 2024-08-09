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
  bool want_recv;
  bool want_send;
} fd_status_t;

const fd_status_t fd_status_R = {
    .want_recv = true,
    .want_send = false,
};
const fd_status_t fd_status_S = {
    .want_recv = false,
    .want_send = true,
};
const fd_status_t fd_status_RS = {
    .want_recv = true,
    .want_send = true,
};
const fd_status_t fd_status_NORS = {
    .want_recv = false,
    .want_send = false,
};

fd_status_t on_peer_connected(int client_sockfd, const struct sockaddr_in* peer_addr, socklen_t peer_addr_len) {
  assert(client_sockfd < MAXFDs);
  report_peer_connected(peer_addr, peer_addr_len);

  peer_state_t* peerState = &global_state[client_sockfd];
  peerState->state = INITIAL_ACK;
  peerState->sendbuf[0] = (uint8_t)'*';
  peerState->sendptr = 0;
  peerState->sendbuf_end = 1;

  return fd_status_S;
}

fd_status_t on_peer_ready_recv(int client_sockfd) {
  assert(client_sockfd < MAXFDs);
  peer_state_t* peerState = &global_state[client_sockfd];

  if (peerState->state == INITIAL_ACK || peerState->sendptr < peerState->sendbuf_end) {
    // Until the initial ACK has been sent to the peer, there's nothing we
    // want to receive. Also, wait until all data staged for sending is sent to
    // receive more data.
    return fd_status_S;
  }
  printf("here\n");

  uint8_t buf[1024];
  int bytesRecv = recv(client_sockfd, buf, sizeof buf, 0);
  if (bytesRecv == 0) {
    // The peer disconnected.
    return fd_status_NORS;
  } else if (bytesRecv == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
    // The client socket is not *really* ready for recv; wait until it is.
    return fd_status_R;
  } else {
    perror_die("recv");
  }

  bool ready_to_send = false;
  for (int i = 0; i < bytesRecv; ++i) {
    switch (peerState->state) {
      case INITIAL_ACK:
        assert(0 && "can't reach here");
        break;
      case WAIT_FOR_MSG:
        if (buf[i] == '^') peerState->state = IN_MSG;
        break;
      case IN_MSG:
        if (buf[i] == '$') {
          peerState->state = WAIT_FOR_MSG;
        } else {
          assert(peerState->sendbuf_end < SENDBUF_SIZE);
          peerState->sendbuf[peerState->sendbuf_end++] = buf[i] + 1;
          ready_to_send = true;
        }
        break;
    }
  }
  // Report reading readiness iff there's nothing to send to the peer as a
  // result of the latest recv.
  return (fd_status_t){
      .want_recv = !ready_to_send,
      .want_send = ready_to_send,
  };
}

fd_status_t on_peer_ready_send(int client_sockfd) {
  assert(client_sockfd < MAXFDs);
  peer_state_t* peerState = &global_state[client_sockfd];

  if (peerState->sendptr >= peerState->sendbuf_end) {
    // Nothing to send.
    return fd_status_RS;
  }
  int msglen = peerState->sendbuf_end - peerState->sendptr;
  int numBytesSent = send(client_sockfd, &peerState->sendbuf[peerState->sendptr], msglen, 0);
  if (numBytesSent == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
      return fd_status_S;
    } else {
      perror_die("send");
    }
  }
  if (numBytesSent < msglen) {
    peerState->sendptr += numBytesSent;
    return fd_status_S;
  } else {
    // Everything was sent successfully; reset the send queue.
    peerState->sendptr = 0;
    peerState->sendbuf_end = 0;

    // Special-case state transition in if we were in INITIAL_ACK until now.
    if (peerState->state == INITIAL_ACK) peerState->state = WAIT_FOR_MSG;

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
  fd_set recvfd_monitor_set;
  FD_ZERO(&recvfd_monitor_set);
  fd_set sendfd_monitor_set;
  FD_ZERO(&sendfd_monitor_set);

  // The server socket is always monitored for recv, to detect when new
  // peer connections are incoming.
  FD_SET(server_sockfd, &recvfd_monitor_set);

  // For more efficiency, fdset_max tracks the maximal FD seen so far; this
  // makes it unnecessary for select to iterate all the way to FD_SETSIZE on
  // every call.
  int fdset_max = server_sockfd;

  while (1) {
    fd_set recvfd_set = recvfd_monitor_set;
    fd_set sendfd_set = sendfd_monitor_set;

    int numOfReady = select(fdset_max + 1, &recvfd_set, &sendfd_set, NULL, NULL);
    if (numOfReady == SOCKET_ERROR) {
      perror_die("[MAIN-LOOP] select error");
    }
    printf("numOfReady: %d\n", numOfReady);

    // numOfReady tells us the total number of ready events; if one socket is both
    // readable and writable it will be 2. Therefore, it's decremented when
    // either a readable or a writable socket is encountered.
    for (int fd = 0; fd <= fdset_max && numOfReady > 0; fd++) {
      // Check if this fd became receivable.
      if (FD_ISSET(fd, &recvfd_set)) {
        numOfReady--;

        if (fd == server_sockfd) {
          // The server socket is ready; this means a new peer is connecting.
          struct sockaddr_in peer_addr;
          socklen_t peer_addr_len = sizeof(peer_addr);
          int client_sockfd = accept(server_sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
          if (client_sockfd == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
              // This can happen due to the nonblocking socket mode; in this
              // case don't do anything, but print a notice (since these events
              // are extremely rare and interesting to observe...)
              printf("accept returned EAGAIN or EWOULDBLOCK\n");
            } else {
              perror_die("accept");
            }
          } else {
            printf("client sockfd: %d\n", client_sockfd);
            make_socket_non_blocking(client_sockfd);
            if (client_sockfd > fdset_max) {
              if (client_sockfd >= FD_SETSIZE) {
                die("socket fd (%d) >= FD_SETSIZE (%d)", client_sockfd, FD_SETSIZE);
              }
              fdset_max = client_sockfd;
            }

            fd_status_t server_status = on_peer_connected(client_sockfd, &peer_addr, peer_addr_len);
            if (server_status.want_recv) {
              FD_SET(client_sockfd, &recvfd_monitor_set);
            } else {
              FD_CLR(client_sockfd, &recvfd_monitor_set);
            }
            if (server_status.want_send) {
              FD_SET(client_sockfd, &sendfd_monitor_set);
            } else {
              FD_CLR(client_sockfd, &sendfd_monitor_set);
            }
          }
        } else {
          printf("recv new message\n");
          fd_status_t server_status = on_peer_ready_recv(fd);
          if (server_status.want_recv) {
            FD_SET(fd, &recvfd_monitor_set);
          } else {
            FD_CLR(fd, &recvfd_monitor_set);
          }
          if (server_status.want_send) {
            FD_SET(fd, &sendfd_monitor_set);
          } else {
            FD_CLR(fd, &sendfd_monitor_set);
          }
          if (!server_status.want_recv && !server_status.want_send) {
            printf("socket %d closing\n", fd);
            closesocket(fd);
          }
        }
      }

      // Check if this fd became sendable.
      if (FD_ISSET(fd, &sendfd_set)) {
        numOfReady--;
        fd_status_t server_status = on_peer_ready_send(fd);
        if (server_status.want_recv) {
          FD_SET(fd, &recvfd_monitor_set);
        } else {
          FD_CLR(fd, &recvfd_monitor_set);
        }
        if (server_status.want_send) {
          FD_SET(fd, &sendfd_monitor_set);
        } else {
          FD_CLR(fd, &sendfd_monitor_set);
        }
        if (!server_status.want_recv && !server_status.want_send) {
          printf("socket %d closing\n", fd);
          closesocket(fd);
        }
      }
    }
  }
  cleanupWinsock();
  return 0;
}
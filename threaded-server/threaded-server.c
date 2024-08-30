#include <stdint.h>
#include <stdio.h>

#include "utils.h"

typedef struct {
  int sockfd;
} thread_config_t;

typedef enum {
  WAIT_FOR_MSG,
  IN_MSG,
} ProcessingState;

void serve_connection(int sockfd) {
  // echo "*" back to client
  if (send(sockfd, "*", 1, 0) < 1) {
    perror_die("[SERVE-CONNECTION] send * die");
  }
  // change state to wait for message
  ProcessingState state = WAIT_FOR_MSG;

  while (1) {
    uint8_t buf[1024];
    int len = recv(sockfd, buf, sizeof(buf), 0);
    if (len == SOCKET_ERROR) {
      perror_die("[SERVE-CONNECTION] recv die");
    } else if (len == 0)
      break;

    for (int i = 0; i < len; i++) {
      switch (state) {
        case WAIT_FOR_MSG:
          if (buf[i] == '^') {
            state = IN_MSG;
          }
          break;
        case IN_MSG:
          if (buf[i] == '$') {
            state = WAIT_FOR_MSG;
          } else {
            buf[i] += 1;
            if (send(sockfd, &buf[i], 1, 0) < 1) {
              perror("[SERVE-CONNECTION] send buf die");
              closesocket(sockfd);
              return;
            }
          }
          break;
        default:
          break;
      }
    }
  }
  closesocket(sockfd);
}

void* server_thread(void* arg) {
  thread_config_t* thread_config = (thread_config_t*)arg;
  int sockfd = thread_config->sockfd;
  free(thread_config);
  pthread_t thread_id = pthread_self();
  // printf("Thread %p created to handle connection with socket %d\n", (void*)thread_id, sockfd);
  serve_connection(sockfd);
  // printf("Thread %p done\n", (void*)thread_id);
  return 0;
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
  printf("Serving on port: %d\n", portnum);

  int sockfd = listen_inet_socket(portnum);
  printf("sockfd: %d\n", sockfd);

  while (1) {
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int newSockFd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);  // return a new connection
    if (newSockFd < 0) {
      perror_die("[MAIN-LOOP] ERROR CONNECTION on accept");
    }
    printf("newSockFd: %d\n", newSockFd);

    report_peer_connected(&peer_addr, peer_addr_len);
    pthread_t the_thread;

    thread_config_t* thread_config = (thread_config_t*)malloc(sizeof(*thread_config));
    if (!thread_config) {
      die("OOM");
    }
    thread_config->sockfd = newSockFd;
    pthread_create(&the_thread, NULL, server_thread, thread_config);

    pthread_detach(the_thread);

    printf("[MAIN-LOOP] PEERING DONE!!!\n");
  }
  cleanupWinsock();
  return 0;
}

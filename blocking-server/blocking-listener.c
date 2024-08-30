#include <stdint.h>
#include <stdio.h>

#include "utils.h"

int main(int argc, const char** argv) {
  if (initializeWinsock() != 0) {
    return 1;
  }

  setvbuf(stdout, NULL, _IONBF, 0);
  int portnum = 9988;
  if (argc >= 2) {
    portnum = atoi(argv[1]);
  }
  printf("Listening on port: %d\n", portnum);

  int sockfd = listen_inet_socket(portnum);
  printf("sockfd: %d\n", sockfd);
  struct sockaddr_in peer_addr;
  socklen_t peer_addr_len = sizeof(peer_addr);

  int newSockFd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
  if (newSockFd < 0) {
    perror_die("[MAIN-LOOP] ERROR CONNECTION on accept");
  }
  printf("newSockFd: %d\n", newSockFd);
  report_peer_connected(&peer_addr, peer_addr_len);

  while (1) {
    uint8_t buf[1024];
    printf("Calling recv...\n");
    int len = recv(newSockFd, buf, sizeof(buf), 0);
    if (len == SOCKET_ERROR) {
      perror_die("recv die");
    } else if (len == 0) {
      printf("Peer disconnected; I'm done.\n");
      break;
    }
    printf("recv returned %d bytes\n", len);
  }
  closesocket(newSockFd);
  closesocket(sockfd);
  cleanupWinsock();
  return 0;
}
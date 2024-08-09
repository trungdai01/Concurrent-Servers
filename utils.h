#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

#pragma comment(lib, "ws2_32.lib")

int initializeWinsock();
void cleanupWinsock();

// Dies (exits with a failure status) after printing the given printf-like
// message to stdout.
void die(char* fmt, ...);

// Wraps malloc with error checking: dies if malloc fails.
void* xmalloc(size_t size);

// Dies (exits with a failure status) after printing the current perror status
// prefixed with msg.
void perror_die(char* msg);

// Reports a peer connection to stdout. sa is the data populated by a successful
// accept() call.
void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen);

// Creates a bound and listening INET socket on the given port number. Returns
// the socket fd when successful; dies in case of errors.
int listen_inet_socket(int portnum);

// Sets the given socket into non-blocking mode.
void make_socket_non_blocking(int sockfd);
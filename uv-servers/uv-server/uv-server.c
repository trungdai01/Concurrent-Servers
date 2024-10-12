#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"

#include "utils.h"

#define N_BACKLOG 64

typedef enum {
    INITIAL_ACK,
    WAIT_FOR_MSG,
    IN_MSG,
} ProcessingState;

#define SENDBUF_SIZE 1024

typedef struct {
    ProcessingState state;
    char send_buf[SENDBUF_SIZE];
    int send_buf_end;
    uv_tcp_t* client;
} peer_state_t;

/// @brief
/// @param handle
/// @param suggested_size 65536 at the moment in most cases
/// @param buf
void on_alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)xmalloc(suggested_size);
    buf->len = suggested_size;
}

void on_client_closed(uv_handle_t* handle) {
    uv_tcp_t* client = (uv_tcp_t*)handle;
    if (client->data) free(client->data);
    free(client);
}

void on_sent_buf(uv_write_t* req, int status) {
    if (status) die("Write error: %s\n", uv_strerror(status));

    peer_state_t* peer_handler = (peer_state_t*)req->data;

    // Kill switch for testing leaks in the server. When a client sends a message
    // ending with WXY (note the shift-by-1 in send_buf), this signals the server
    // to clean up and exit, by stopping the default event loop. Running the
    // server under valgrind can now track memory leaks, and a run should be
    // clean except a single uv_tcp_t allocated for the client that sent the kill
    // signal (it's still connected when we stop the loop and exit).
    if (peer_handler->send_buf_end >= 3 && peer_handler->send_buf[peer_handler->send_buf_end - 3] == 'X' &&
        peer_handler->send_buf[peer_handler->send_buf_end - 2] == 'Y' &&
        peer_handler->send_buf[peer_handler->send_buf_end - 1] == 'Z') {
        free(peer_handler);
        free(req);
        uv_stop(uv_default_loop());
        return;
    }

    peer_handler->send_buf_end = 0;
    free(req);
}


/// @brief Note: Must be responsible for freeing the buffer 
/// @param client 
/// @param nread data available status
/// @param buf
void on_received_message(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if (nread != UV_EOF) fprintf(stderr, "Receive error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t*)client, on_client_closed);
    } else if (nread > 0) {
        assert(buf->len >= nread);
        peer_state_t* peer_handler = (peer_state_t*)client->data;
        if (peer_handler->state == INITIAL_ACK) {
            free(buf->base);
            return;
        }

        for (int i = 0; i < nread; ++i) {
            switch (peer_handler->state) {
                case WAIT_FOR_MSG:
                    if (buf->base[i] == '^') peer_handler->state = IN_MSG;
                    break;
                case IN_MSG:
                    if (buf->base[i] == '$') {
                        peer_handler->state = WAIT_FOR_MSG;
                    } else {
                        assert(peer_handler->send_buf_end < SENDBUF_SIZE);
                        peer_handler->send_buf[peer_handler->send_buf_end++] = buf->base[i] + 1;
                    }
                    break;
                default:
                    break;
            }
        }

        if (peer_handler->send_buf_end > 0) {
            uv_buf_t send_buf = uv_buf_init(peer_handler->send_buf, peer_handler->send_buf_end);
            uv_write_t* send_req = (uv_write_t*)xmalloc(sizeof(*send_req));
            send_req->data = peer_handler;
            int return_code = uv_write(send_req, (uv_stream_t*)client, &send_buf, 1, on_sent_buf);
            if (return_code < 0) die("[on_received_message] uv_write failed: %s", uv_strerror(return_code));
        }
    }
    free(buf->base);
}

void on_sent_init_ack(uv_write_t* req, int status) {
    if (status) die("Write init ack error: %s\n", uv_strerror(status));

    peer_state_t* peer_handler = (peer_state_t*)req->data;
    peer_handler->state = WAIT_FOR_MSG;
    peer_handler->send_buf_end = 0;

    int return_code = uv_read_start((uv_stream_t*)peer_handler->client, on_alloc_buffer, on_received_message);
    if (return_code < 0) die("[ON_SENT_INIT_ACK] uv_read_start failed: %s", uv_strerror(return_code));

    free(req);
}

void on_peer_connected(uv_stream_t* server_stream, int status) {
    if (status < 0) {
        fprintf(stderr, "Peer connection error: %s\n", uv_strerror(status));
        return;
    }

    // A TCP client will represent the connected peer; it's allocated on the heap and only
    // released when the client disconnects. This client holds a pointer to
    // peer_state_t in its data field; this peer state tracks the protocol state
    // with this client throughout interaction.
    uv_tcp_t* client = (uv_tcp_t*)xmalloc(sizeof(*client));
    int return_code = uv_tcp_init(uv_default_loop(), client);
    if (return_code < 0) die("[ON_PEER_CONNECTED] uv_tcp_init failed: %s", uv_strerror(return_code));

    client->data = NULL;

    // uv_accept is used in conjunction with uv_listen() to accept incoming connections.
    if (uv_accept(server_stream, (uv_stream_t*)client) == 0) {
        struct sockaddr_storage peer_name;
        int name_len = sizeof(peer_name);
        // Copy the address of connected peer to the client.
        return_code = uv_tcp_getpeername(client, (struct sockaddr*)&peer_name, &name_len);
        if (return_code < 0) die("[ON_PEER_CONNECTED] uv_tcp_getpeername failed: %s", uv_strerror(return_code));

        report_peer_connected((const struct sockaddr_in*)&peer_name, name_len);

        peer_state_t* peer_handler = (peer_state_t*)xmalloc(sizeof(*peer_handler));
        peer_handler->state = INITIAL_ACK;
        peer_handler->send_buf[0] = '*';
        peer_handler->send_buf_end = 1;
        peer_handler->client = client;

        client->data = peer_handler;

        uv_buf_t send_buf = uv_buf_init(peer_handler->send_buf, peer_handler->send_buf_end);
        uv_write_t* send_req = (uv_write_t*)xmalloc(sizeof(*send_req));
        send_req->data = peer_handler;
        return_code = uv_write(send_req, (uv_stream_t*)client, &send_buf, 1, on_sent_init_ack);
        if (return_code < 0) die("[ON_PEER_CONNECTED] uv_write failed: %s", uv_strerror(return_code));

    } else {
        uv_close((uv_handle_t*)client, on_client_closed);
    }
}

int main(int argc, const char** argv) {
    if (initializeWinsock() != 0) return 1;

    setvbuf(stdout, NULL, _IONBF, 0);

    int portnum = 9090;
    if (argc >= 2) portnum = atoi(argv[1]);

    printf("[MAIN] Serving on port %d\n", portnum);

    int return_code;
    uv_tcp_t server_stream;
    return_code = uv_tcp_init(uv_default_loop(), &server_stream);
    if (return_code < 0) die("[MAIN] uv_tcp_init failed: %s", uv_strerror(return_code));

    struct sockaddr_in server_address;
    const char* ip = "0.0.0.0";
    // uv_ip4_addr : Convert a string containing an IPV4 addresses to a binary structure
    return_code = uv_ip4_addr(ip, portnum, &server_address);
    if (return_code < 0) die("[MAIN] uv_ip4_addr failed: %s", uv_strerror(return_code));

    // uv_tcp_bind : Bind server_stream to the address above. flags is used to work with IPV6
    return_code = uv_tcp_bind(&server_stream, (const struct sockaddr*)&server_address, 0);
    if (return_code < 0) die("[MAIN] uv_tcp_bind failed: %s", uv_strerror(return_code));

    // Start listening for incoming connections. backlog indicates the number of connections
    // the kernel might queue. When a new incoming connection is received, on_peer_connected is invoked.
    return_code = uv_listen((uv_stream_t*)&server_stream, N_BACKLOG, on_peer_connected);
    if (return_code < 0) die("[MAIN] uv_listen failed: %s", uv_strerror(return_code));

    // Run the libuv event loop.
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    cleanupWinsock();
    // If uv_run returned, close the default loop before exiting.
    return uv_loop_close(uv_default_loop());
}
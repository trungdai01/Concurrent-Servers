import argparse
import logging
import socket
import threading
import time


class ReadThread(threading.Thread):
    def __init__(self, name, sockobj):
        super().__init__()
        self.sockobj = sockobj
        self.name = name
        self.bufsize = 8 * 1024

    def run(self):
        fullbuf = b""
        while True:
            recv_buf = self.sockobj.recv(self.bufsize)
            logging.info("{%s} received {%s}", self.name, recv_buf)
            fullbuf += recv_buf
            if b"1111" in fullbuf:
                break


def make_new_connection(name, host, port):
    """Creates a single socket connection to the host:port.

    Sets a pre-set sequence of messages to the server with pre-set delays; in
    parallel, reads from the socket in a separate thread.
    """
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))
    if client_socket.recv(1) != b"*":
        logging.error("Something is wrong! Did not receive *")
    logging.info("{%s} connected...", name)

    rthread = ReadThread(name, client_socket)
    rthread.start()

    message = b"^abc$de^abte$f"
    logging.info("{%s} sending {%s}", name, message)
    client_socket.send(message)
    time.sleep(1.0)

    message = b"xyz^123"
    logging.info("{%s} sending {%s}", name, message)
    client_socket.send(message)
    time.sleep(1.0)

    # The 0000 sent to the server here will result in an echo of 1111, which is
    # a sign for the reading thread to terminate.
    # Add WXY after 0000 to enable kill-switch in some servers.
    message = b"25$^ab0000$abab"
    logging.info("{%s} sending {%s}", name, message)
    client_socket.send(message)
    time.sleep(0.2)

    rthread.join()
    client_socket.close()
    logging.info("{%s} disconnecting", name)


def main():
    argparser = argparse.ArgumentParser("Simple TCP client")
    argparser.add_argument("host", help="Server host name")
    argparser.add_argument("port", type=int, help="Server port")
    argparser.add_argument("-n", "--num_concurrent", type=int, default=1, help="Number of concurrent connections")
    args = argparser.parse_args()

    logging.basicConfig(level=logging.DEBUG, format="%(levelname)s:%(asctime)s:%(message)s")

    t1 = time.time()
    connections = []
    for i in range(args.num_concurrent):
        name = f"conn{i}"
        tconn = threading.Thread(target=make_new_connection, args=(name, args.host, args.port))
        tconn.start()
        connections.append(tconn)

    for conn in connections:
        conn.join()

    print("Elapsed:", time.time() - t1)


if __name__ == "__main__":
    main()

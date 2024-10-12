import argparse
from concurrent.futures import ThreadPoolExecutor
from enum import Enum
import socket
import sys

ProcessingState = Enum("ProcessingState", "WAIT_FOR_MSG IN_MSG")


def serve_connection(sock_obj: socket, client_address):
    print(f"{client_address} connected")
    sock_obj.sendall(b"*")
    state = ProcessingState.WAIT_FOR_MSG

    while True:
        try:
            buf = sock_obj.recv(1024)
            if not buf:
                break
        except IOError:
            break
        for byte in buf:
            if state == ProcessingState.WAIT_FOR_MSG:
                if byte == ord(b"^"):
                    state = ProcessingState.IN_MSG
            elif state == ProcessingState.IN_MSG:
                if byte == ord(b"$"):
                    state = ProcessingState.WAIT_FOR_MSG
                else:
                    sock_obj.send(bytes([byte + 1]))
            else:
                assert False

    print(f"{client_address} done")
    sys.stdout.flush()
    sock_obj.close()


if __name__ == "__main__":
    argparser = argparse.ArgumentParser("Threadpool server")
    argparser.add_argument("--port", type=int, default=9090, help="Server port")
    argparser.add_argument("-n", type=int, default=64, help="Number of threads in pool")
    args = argparser.parse_args()

    pool = ThreadPoolExecutor(args.n)
    sock_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_obj.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock_obj.bind(("localhost", args.port))
    sock_obj.listen(15)

    try:
        while True:
            client_socket, client_address = sock_obj.accept()
            pool.submit(serve_connection, client_socket, client_address)
    except KeyboardInterrupt as exception:
        print(exception)
        sock_obj.close()

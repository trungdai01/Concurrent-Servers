import argparse
from concurrent.futures import ThreadPoolExecutor
from enum import Enum
import socket
import sys

ProcessingState = Enum("ProcessingState", "WAIT_FOR_MSG IN_MSG")


def serve_connection(sockObj: socket, clientAddress):
    print(f"{clientAddress} connected")
    sockObj.sendall(b"*")
    state = ProcessingState.WAIT_FOR_MSG

    while True:
        try:
            buf = sockObj.recv(1024)
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
                    sockObj.send(bytes([byte + 1]))
            else:
                assert False

    print(f"{clientAddress} done")
    sys.stdout.flush()
    sockObj.close()


if __name__ == "__main__":
    argparser = argparse.ArgumentParser("Threadpool server")
    argparser.add_argument("--port", type=int, default=9090, help="Server port")
    argparser.add_argument("-n", type=int, default=64, help="Number of threads in pool")
    args = argparser.parse_args()

    pool = ThreadPoolExecutor(args.n)
    sockobj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockobj.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sockobj.bind(("localhost", args.port))
    sockobj.listen(15)

    try:
        while True:
            client_socket, client_address = sockobj.accept()
            pool.submit(serve_connection, client_socket, client_address)
    except KeyboardInterrupt as exception:
        print(exception)
        sockobj.close()

import socket
import threading

PORT = 8080
SERVER = "192.168.1.5"
ADDR = (SERVER, PORT)

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)

connections = []
lock = threading.Lock()

def broadcast(msg, sender_conn):
    with lock:
        for conn, addr in connections:
            if conn is not sender_conn:
                try:
                    conn.sendall(msg)
                except:
                    pass

def receive_message(conn, addr):
    while True:
        try:
            msg = conn.recv(64)
            if msg:
                broadcast(msg, conn)
        except:
            with lock:
                connections.remove((conn, addr))
            break

def listen_connection():
    while True:
        try:
            server.listen()
            conn, addr = server.accept()
            print(f'[NEW CONNECTION] {addr} connected.')
            with lock:
                connections.append((conn, addr))
            receive_thread = threading.Thread(target=receive_message, args=(conn, addr))
            receive_thread.start()
        except:
            pass

print("[STARTING] Starting Server...")
listen_connection_thread = threading.Thread(target=listen_connection)
listen_connection_thread.start()

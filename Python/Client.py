import socket
import threading
from Crypto.Cipher import AES


PORT = 8080
SERVER = "192.168.1.5"
ADDR = (SERVER, PORT)
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)

# 16-byte secret key for AES encryption/decryption
key = b'0123456789abcdef'

# padding function for the message
pad = lambda s: s + (AES.block_size - len(s) % AES.block_size) * chr(AES.block_size - len(s) % AES.block_size)

# encryption function
encrypt = lambda message: AES.new(key, AES.MODE_ECB).encrypt(pad(message).encode())

# decryption function
decrypt = lambda message: AES.new(key, AES.MODE_ECB).decrypt(message).decode().rstrip()


stop_event = threading.Event()


def receive():
    while not stop_event.is_set():
        message = client.recv(1024)
        decrypted_message = decrypt(message)
        print(f"{decrypted_message}")


def send():
    while not stop_event.is_set():
        message = input("")

        encrypted_message = encrypt(message)
        client.send(encrypted_message)


receive_thread = threading.Thread(target=receive)
receive_thread.start()
send_thread = threading.Thread(target=send)
send_thread.start()

try:
    while not stop_event.is_set():
        pass
except KeyboardInterrupt:
    stop_event.set()
    client.close()
    receive_thread.join()
    send_thread.join()

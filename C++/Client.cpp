#include <iostream>
#include <cstring>
#include <thread>
#include <openssl/aes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 8080
#define SERVER "192.168.1.6"

// 16-byte secret key for AES encryption/decryption
unsigned char key[] = "0123456789abcdef";

// padding function for the message
string pad(const string& s) {
    size_t block_size = AES_BLOCK_SIZE;
    size_t pad_len = block_size - s.length() % block_size;
    char pad_char = static_cast<char>(pad_len);
    string padded = s;
    padded.append(pad_len, pad_char);
    return padded;
}

// encryption function
string encrypt(const string& message) {
    string padded_message = pad(message);
    size_t message_length = padded_message.length();
    unsigned char encrypted_message[message_length];
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);
    for (size_t i = 0; i < message_length; i += AES_BLOCK_SIZE) {
        AES_encrypt((const unsigned char*)padded_message.c_str() + i,
                    encrypted_message + i, &aes_key);
    }
    return string((char*)encrypted_message, message_length);
}

// decryption function
string decrypt(const string& message) {
    size_t message_length = message.length();
    unsigned char decrypted_message[message_length + AES_BLOCK_SIZE];  // allocate buffer of max possible size
    AES_KEY aes_key;
    AES_set_decrypt_key(key, 128, &aes_key);
    for (size_t i = 0; i < message_length; i += AES_BLOCK_SIZE) {
        AES_decrypt((const unsigned char*)message.c_str() + i,
                    decrypted_message + i, &aes_key);
    }
    size_t pad_len = decrypted_message[message_length + AES_BLOCK_SIZE - 1];  // retrieve pad length from last byte
    size_t actual_length = message_length - pad_len;  // calculate actual length of decrypted message
    return string((char*)decrypted_message, actual_length);  // return decrypted message with actual length
}

char hostname[1024];
string NAME = "client";
int client_fd;

string receive() {
    char buffer[2048];
    int valread = read(client_fd, buffer, sizeof(buffer));
    string message(buffer, valread);
    string decrypted_message = decrypt(message);
    return decrypted_message;
}

void send_messages() {
    while (true) {
        string message;
        getline(cin, message);
        message = "[" + NAME + "]: " + message;
        string encrypted_message = encrypt(message);
        send(client_fd, encrypted_message.c_str(), encrypted_message.length(), 0);
    }
}

void client_thread() {
    struct sockaddr_in server_addr;
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER, &server_addr.sin_addr);
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        cerr << "Connection Failed" << endl;
        exit(1);
    }

    while (true) {
        string message = receive();
        cout << message << endl;
    }
}

int main() {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    gethostname(hostname, 1024);
    NAME = hostname;

    thread client(client_thread);
    thread sender(send_messages);

    client.join();
    sender.join();

    close(client_fd);
    return 0;
}
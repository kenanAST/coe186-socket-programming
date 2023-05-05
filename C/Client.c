#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/aes.h>

#define PORT 8080
#define SERVER "192.168.1.6"

// 16-byte secret key for AES encryption/decryption
unsigned char key[] = "0123456789abcdef";

// padding function for the message
char *pad(const char *s) {
    size_t block_size = AES_BLOCK_SIZE;
    size_t pad_len = block_size - strlen(s) % block_size;
    char pad_char = (char)pad_len;
    char *padded = malloc(strlen(s) + pad_len + 1);
    strcpy(padded, s);
    memset(padded + strlen(s), pad_char, pad_len);
    padded[strlen(s) + pad_len] = '\0';
    return padded;
}

// encryption function
char *encrypt_message(const char *message) {
    char *padded_message = pad(message);
    size_t message_length = strlen(padded_message);
    unsigned char encrypted_message[message_length];
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);
    for (size_t i = 0; i < message_length; i += AES_BLOCK_SIZE) {
        AES_encrypt((const unsigned char*)padded_message + i,
                    encrypted_message + i, &aes_key);
    }
    char *result = malloc(message_length + 1);
    memcpy(result, encrypted_message, message_length);
    result[message_length] = '\0';
    free(padded_message);
    return result;
}

// decryption function
char *decrypt_message(const char *message) {
    size_t message_length = strlen(message);
    unsigned char decrypted_message[message_length];
    AES_KEY aes_key;
    AES_set_decrypt_key(key, 128, &aes_key);
    for (size_t i = 0; i < message_length; i += AES_BLOCK_SIZE) {
        AES_decrypt((const unsigned char*)message + i,
                    decrypted_message + i, &aes_key);
    }
    size_t pad_len = decrypted_message[message_length - 1];
    char *result = malloc(message_length - pad_len + 1);
    memcpy(result, decrypted_message, message_length - pad_len);
    result[message_length - pad_len] = '\0';
    return result;
}

char hostname[1024];
char *NAME = "client";
int client_fd;

char *receive() {
    char buffer[2048];
    int valread = read(client_fd, buffer, sizeof(buffer));
    char *message = malloc(valread + 1);
    memcpy(message, buffer, valread);
    message[valread] = '\0';
    char *decrypted_message = decrypt_message(message);
    free(message);
    return decrypted_message;
}

void send_messages() {
    while (true) {
        char message[2048];
        fgets(message, sizeof(message), stdin);
        message[strlen(message) - 1] = '\0';  // remove the newline character
        char *formatted_message = malloc(strlen(message) + strlen(NAME) + 4);
        sprintf(formatted_message, "[%s]: %s", NAME, message);
        char *encrypted_message = encrypt_message(formatted_message);
        send(client_fd, encrypted_message, strlen(encrypted_message), 0);
        free(formatted_message);
        free(encrypted_message);
    }
}

void client_thread() {
    struct sockaddr_in server_addr;
    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER, &server_addr.sin_addr);
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "Connection Failed\n");
        exit(1);
    }

    while (1) {
        char buffer[2048];
        int valread = read(client_fd, buffer, sizeof(buffer));
        char* message = decrypt_message(buffer);
        printf("%s\n", message);
        free(message);
    }
}


int main() {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        fprintf(stderr, "Socket creation failed\n");
        return 1;
    }

    gethostname(hostname, 1024);
    NAME = hostname;

    pthread_t client_thread_id;
    pthread_t sender_thread_id;
    pthread_create(&client_thread_id, NULL, client_thread, NULL);
    pthread_create(&sender_thread_id, NULL, send_messages, NULL);
    pthread_join(client_thread_id, NULL);
    pthread_join(sender_thread_id, NULL);

    close(client_fd);
    return 0;
}
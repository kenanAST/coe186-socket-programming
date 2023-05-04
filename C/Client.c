#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/aes.h>

#define PORT 8080
#define SERVER "192.168.1.5"
#define KEY_SIZE 128

int client_fd;
pthread_t receive_thread, send_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int stop_event = 0;
AES_KEY aes_key;

// Encrypt plaintext using AES-128 ECB mode
void aes_encrypt(char *plaintext, int plaintext_len, unsigned char *ciphertext) {
    AES_ecb_encrypt((unsigned char*)plaintext, ciphertext, &aes_key, AES_ENCRYPT);
}

// Decrypt ciphertext using AES-128 ECB mode
void aes_decrypt(unsigned char *ciphertext, int ciphertext_len, char *plaintext) {
    AES_ecb_encrypt(ciphertext, (unsigned char*)plaintext, &aes_key, AES_DECRYPT);
}

void *receive_msg(void *arg) {
    while (!stop_event) {
        char buffer[2048];
        memset(buffer, 0, sizeof(buffer));
        int recv_len = recv(client_fd, buffer, sizeof(buffer), 0);
        if (recv_len < 0) {
            stop_event = 1;
            break;
        } else if (recv_len == 0) {
            continue;
        }
        int num_blocks = recv_len / 16;
        if (recv_len % 16 != 0) {
            num_blocks++;
        }
        char *decrypted = malloc(recv_len);
        memset(decrypted, 0, recv_len);
        for (int i = 0; i < num_blocks; i++) {
            unsigned char block[16];
            memset(block, 0, sizeof(block));
            int block_len = (i == num_blocks - 1) ? recv_len % 16 : 16;
            memcpy(block, buffer + i * 16, block_len);
            aes_decrypt(block, 16, decrypted + i * 16);
        }
        printf("%s\n", decrypted);
        free(decrypted);
    }
    pthread_exit(NULL);
}




void *send_msg(void *arg) {
    while (!stop_event) {
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        size_t num_blocks = (len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
        char *encrypted = malloc(num_blocks * AES_BLOCK_SIZE);
        memset(encrypted, 0, num_blocks * AES_BLOCK_SIZE);
        for (size_t i = 0; i < num_blocks; i++) {
            char block[AES_BLOCK_SIZE];
            memset(block, 0, AES_BLOCK_SIZE);
            size_t block_len = i < num_blocks - 1 ? AES_BLOCK_SIZE : len % AES_BLOCK_SIZE;
            memcpy(block, buffer + i * AES_BLOCK_SIZE, block_len);
            aes_encrypt(block, AES_BLOCK_SIZE, (unsigned char *)(encrypted + i * AES_BLOCK_SIZE));
        }
        pthread_mutex_lock(&mutex);
        int send_len = send(client_fd, encrypted, num_blocks * AES_BLOCK_SIZE, 0);
        if (send_len < 0) {
            stop_event = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        free(encrypted);
    }
    pthread_exit(NULL);
}




int main() {
    struct sockaddr_in server_addr;

    char *key = "0123456789abcdef";
    if (AES_set_encrypt_key((unsigned char*)key, KEY_SIZE, &aes_key) < 0) {
        perror("Failed to set encryption key");
        exit(EXIT_FAILURE);
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER);
    server_addr.sin_port = htons(PORT);

    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    pthread_create(&receive_thread, NULL, receive_msg, NULL);
    pthread_create(&send_thread, NULL, send_msg, NULL);

    pthread_join(receive_thread, NULL);
    pthread_join(send_thread, NULL);

    close(client_fd);

    return 0;
}

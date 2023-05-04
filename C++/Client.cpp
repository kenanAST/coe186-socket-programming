#include <iostream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 8080
#define SERVER "192.168.1.5"

int client_fd;
pthread_t receive_thread, send_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int stop_event = 0;

void *receive(void *arg) {
    while (!stop_event) {
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        cout << endl;
        int recv_len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            cout << buffer;
        } else {
            stop_event = 1;
            break;
        }
    }
    pthread_exit(NULL);
}

void *send_msg(void *arg) {
    while (!stop_event) {
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        cout << "Enter message: ";
        fgets(buffer, sizeof(buffer), stdin);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        len = strlen(buffer);
        pthread_mutex_lock(&mutex);
        int send_len = send(client_fd, buffer, len, 0);
        if (send_len < 0) {
            stop_event = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER);
    server_addr.sin_port = htons(PORT);

    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    pthread_create(&receive_thread, NULL, receive, NULL);
    pthread_create(&send_thread, NULL, send_msg, NULL);

    while (!stop_event) {}

    pthread_cancel(receive_thread);
    pthread_cancel(send_thread);

    close(client_fd);

    return 0;
}
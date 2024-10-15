#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_PAYLOAD_SIZE 1024

typedef struct {
    char payload[MAX_PAYLOAD_SIZE];
    time_t send_time;
} PingRequest;

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 1); // Listen for a single client

    printf("Waiting for a client to connect...\n");

    addr_size = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);

    if (client_socket >= 0) {
        printf("Client connected.\n");

        while (1) {
            PingRequest request;
            ssize_t bytes_received = recv(client_socket, &request, sizeof(request), 0);

            if (bytes_received <= 0) {
                printf("Client disconnected.\n");
                break;
            }

            // Log the incoming ping request
            time_t current_time = time(NULL);
            double rtt = difftime(current_time, request.send_time);
            printf("Received ping request, RTT: %.3lf seconds\n", rtt);

            // Send an acknowledgment
            send(client_socket, &request, sizeof(request), 0);
        }

        close(client_socket);
    }

    close(server_socket);
    return 0;
}


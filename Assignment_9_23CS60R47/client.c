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
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Connected to the server.\n");

    while (1) {
        PingRequest request;
        time_t send_time = time(NULL);
        request.send_time = send_time;
        strcpy(request.payload, "PING");
        
        send(client_socket, &request, sizeof(request), 0);

        PingRequest response;
        recv(client_socket, &response, sizeof(response), 0);

        time_t receive_time = time(NULL);
        double rtt = difftime(receive_time, send_time);

        printf("Received acknowledgment with payload: %s, RTT: %.3lf seconds\n", response.payload, rtt);

        sleep(1); // Control the ping interval
    }

    close(client_socket);
    return 0;
}


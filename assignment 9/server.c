#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

#define PORT 8090 

// Client connection handler 
void *connection_handler(void *,pthread_t *); 
void *thread_func(void *args);

// Function to send list output
void send_list(int);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    
    // Create socket 
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
   
    // Bind to port 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );

    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }
    
    // Listen for connections 
    listen(socket_desc , 3);
    
    // Accept incoming connections
    puts("Waiting for connections ...");
    c = sizeof(struct sockaddr_in);
    
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
       puts("Connection accepted");
       
       // Handle each client in a separate thread 
       pthread_t sniffer_thread;
       new_sock = malloc(1);
       *new_sock = client_sock;
       int *psock = malloc(sizeof(int)); 
        *psock = client_sock;
       
       if( pthread_create(&sniffer_thread, NULL, thread_func, (void*)psock )< 0) {
            perror("Could not create thread");
            return 1;
       }
    }
    
    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }
    
    return 0;
}
void *thread_func(void *args) {

  pthread_t *thread_id = (pthread_t *)args;
  
  connection_handler(args, thread_id); 

}
// Client handler 
void *connection_handler(void *socket_desc, pthread_t *thread_id) {
    int sock = *(int*)socket_desc;
    int read_size;
    int bytes_read;
    char buffer[1024];
    char client_message[2000];
    pthread_t *sniffer_thread;
    sniffer_thread = (pthread_t*) thread_id;
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        
        // Got a message from client
        printf("Received: %s\n", client_message);
        
        // Parse message
        char *command = strtok(client_message, " ");
        char *filename = strtok(NULL, " ");
        
        if(strcmp(command, "get") == 0) {
            // Implement get logic
            FILE *fp = fopen(filename, "rb");
            if(fp == NULL) {
                printf("Error opening file for reading\n");
                continue;
            }

            while(1) {
                bytes_read = fread(buffer, 1, 1024, fp);
                if(bytes_read <= 0) {
                break; 
                }
                send(sock, buffer, bytes_read, 0);
            }

            printf("File downloaded successfully\n");
            fclose(fp);
        }
        else if(strcmp(command, "put") == 0) {
           // Implement put logic
           FILE *fp = fopen(filename, "wb");
            if(fp == NULL) {
                printf("Error opening file for writing\n");
                continue; 
            }

            while(1) {
                bytes_read = recv(sock, buffer, 1024, 0);
                if(bytes_read <= 0) {
                break;
                }
                // Check for the "EOF" marker
                if (memcmp(buffer, "EOF", 3) == 0) {
                    break; // Exit the loop when the "EOF" marker is received
                }
                // Debugging: Print received data
                printf("Received %zd bytes: %s\n", (size_t)bytes_read, buffer);
                fwrite(buffer, 1, bytes_read, fp);
                printf("Hello");
            }

            printf("File uploaded successfully\n");
            fclose(fp); 
        }
        else if(strcmp(command, "ls") == 0) {
            send_list(sock);
        }
        else if (strcmp(command, "cd") == 0) {
            if (chdir(filename) == 0) {
                send(sock, "Directory changed successfully.\n", 31, 0);
            } else {
                send(sock, "Failed to change directory.\n", 29, 0);
            }
        }
        else if(strcmp(command, "close") == 0) {

            shutdown(sock, SHUT_RDWR);  
            close(sock);

            pthread_cancel(*sniffer_thread);

            free(socket_desc);
            pthread_exit(NULL);
        }
        // Send response
        char server_response[2000];
        strcpy(server_response,"OK\n");
        send(sock , server_response , strlen(server_response) , 0 );
    }
    
    // Close connection 
    close(sock); 
    free(socket_desc);
    
    return 0;
}

// Send list output
void send_list(int sock) {
    char buffer[1024];
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    
    while ((dir = readdir(d)) != NULL) {
        strcpy(buffer, dir->d_name);
        strcat(buffer, "\n");
        send(sock, buffer, strlen(buffer), 0);    
    }
    closedir(d);
}
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8090

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    
    // Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
   
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( PORT );

    // Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
   
    // Send commands 
    while(1) {
        printf("ftp_client> ");
        fgets(message, sizeof(message), stdin);
        message[strlen(message) - 1] = '\0';
        
        // Send some data
        if( send(sock , message , strlen(message) , 0) < 0) {
            puts("Send failed");
            return 1;
        }
        char *command = strtok(message, " ");
        char *filename = strtok(NULL, " ");
        // Check for the "put" command and send the file
        if (strcmp(command, "put") == 0)
        {
            char filename[256];
            printf("Enter the name of the file to upload: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strlen(filename) - 1] = '\0';

            FILE *file = fopen(filename, "rb");
            if (file == NULL)
            {
                puts("Error opening the file");
                continue;
            }

            char buffer[1024];
            size_t bytesRead;
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
            {
                if (send(sock, buffer, bytesRead, 0) < 0)
                {
                    puts("Send failed");
                    break;
                }
            }
            // After sending the file data, send an "EOF" marker to signal the end of file transmission
            const char *eof_marker = "EOF";
            send(sock, eof_marker, strlen(eof_marker), 0);

            fclose(file);
        }
        
        if (strcmp(command, "cd") == 0) {
            // Change the client's working directory
            if (chdir(filename) == 0) {
                printf("Directory changed to: %s\n", filename);
            } else {
                printf("Failed to change directory to: %s\n", filename);
            }
        }
        
        // Receive reply 
        if( recv(sock , server_reply , 2000 , 0) < 0) {
            puts("recv failed");
            break;
        }
        
        puts(server_reply);

        // Check for close command
        if(strcmp(message, "close") == 0) {
            close(sock);
            return 0; 
        }
    }
   
    close(sock);
    return 0;
}
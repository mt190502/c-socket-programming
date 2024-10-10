#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10             // Define the maximum number of pending connections that the server will allow
#define BUFFER_SIZE 1024       // Define the size of the buffer that the server will use to receive messages
#define SERVER_IP "127.0.0.1"  // Define the IP address that the server will listen on
#define SERVER_PORT 8080       // Define the port number that the server will listen on

int main(void) {
    int server_fd, client_fd;                     // Define file descriptors for the server and client sockets
    struct sockaddr_in source_addr, client_addr;  // Define structs for the server and client addresses
    socklen_t client_addr_len;                    // Define the length of the client address

    // Create a socket for the server
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("error: socket option failed, aborting...");
        return 1;
    }

    // Set the source server address
    source_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    source_addr.sin_port = htons(SERVER_PORT);
    source_addr.sin_family = PF_INET;

    // Bind the server address to the server socket
    if ((bind(server_fd, (struct sockaddr*)&source_addr, sizeof(source_addr))) == -1) {
        perror("error: socket binding failed, aborting...");
        return 1;
    }

    printf("server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    // Listen for incoming connections
    while (1) {
        if ((listen(server_fd, BACKLOG)) == -1) {
            perror("error: socket listening failed, aborting...");
            return 1;
        }

        // Accept incoming connections
        client_addr_len = sizeof(client_addr);
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
            perror("error: socket accepting failed, aborting...");
            return 1;
        }

        // Receive messages from the client
        char buffer[BUFFER_SIZE];
        int bytes_received;
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("received message from %s:%d (%4d byte): %s\n", inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port), bytes_received, buffer);

            // Echo the message back to the client
            if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                perror("error: socket sending failed, aborting...");
                return 1;
            } else {
                printf("     reply message to %s:%d (%4d byte): %s\n", inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port), bytes_received, buffer);
            }
        }

        // Close the client socket
        close(client_fd);
    }

    return 0;
}
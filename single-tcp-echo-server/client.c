#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024       // Define the size of the buffer that the client will use to send messages
#define SERVER_IP "127.0.0.1"  // Define the IP address that the client will connect to
#define SERVER_PORT 8080       // Define the port number that the client will connect to

void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
            printf("\nreceived SIGINT, exiting...\n");
            exit(0);
        default:
            break;
    }
}

int main(void) {
    int client_fd;                   // Define a file descriptor for the client socket
    struct sockaddr_in server_addr;  // Define a struct for the server address

    // Create a socket for the client
    if ((client_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return 1;
    }

    // Set the dest server address
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = PF_INET;

    // Connect to the server
    if ((connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
        perror("error: socket connection failed, aborting...");
        return 1;
    }

    // send and receive messages from the server
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while (1) {
        printf("client> ");

        // read input from the stdin
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (feof(stdin)) {
                printf("EOF\n");
                break;
            }
            perror("error: reading from stdin failed, aborting...");
            return 1;
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) {
            continue;
        }
        if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
            perror("error: socket sending failed, aborting...");
            return 1;
        }
        if ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("server> %s\n", buffer);
        }
    }
    close(client_fd);
}

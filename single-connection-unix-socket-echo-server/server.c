#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define BACKLOG 3                                   //- Maximum number of pending connections (if linux, you can set it to SOMAXCONN)
#define BUFFER_SIZE 1024                            //- Message buffer size
#define SERVER_SOCKET_FILE "/tmp/echo_server.sock"  //- Server socket file path

void sig_handler(int sig);

int main(void) {
    int server_fd, client_fd;                     //- Define a file descriptor for the server socket
    struct sockaddr_un server_addr, client_addr;  //- Define a struct for the server address
    char buffer[BUFFER_SIZE];                     //- Define a buffer to store the received message
    ssize_t bytes_received;                       //- Define a variable to store the size of the received message

    struct sigaction sa;            //- Define a struct for the signal handler
    sa.sa_handler = sig_handler;    //- Set the signal handler function
    sa.sa_flags = 0;                //- Set the flags to 0 (flags are not used in this example)
    sigaction(SIGABRT, &sa, NULL);  //- Register the signal handler for SIGABRT
    sigaction(SIGINT, &sa, NULL);   //- Register the signal handler for SIGINT
    sigaction(SIGKILL, &sa, NULL);  //- Register the signal handler for SIGKILL
    sigaction(SIGTERM, &sa, NULL);  //- Register the signal handler for SIGTERM

    //* Create a socket for the server
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, AF_UNIX, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for stream-oriented sockets.
    //- The 3rd argument, 0, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((server_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("error: socket creation failed, aborting...");
        return EXIT_FAILURE;
    }

    //* Set the source server address
    //- The memset() function fills the server_addr struct with zeros.
    //- The sun_family field will be set to AF_UNIX, which specifies the address family of the socket.
    //- The sun_path field will be set to SERVER_SOCKET_FILE, which specifies the path of the server socket.
    //- The unlink() function removes the server socket file if it already exists.
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_FILE, sizeof server_addr.sun_path - 1);

    //* Bind the server address to the server socket
    //- The bind() syscall binds the server address to the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, (struct sockaddr*)&server_addr, specifies the server address.
    //- The 3rd argument, sizeof(server_addr), specifies the size of the server address.
    //? If the bind() syscall fails, it returns -1.
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof server_addr) == -1) {
        if (errno == EADDRINUSE) {
            printf("error: socket binding failed, address already in use, aborting...\n");
            close(server_fd);
            return EXIT_FAILURE;
        } else {
            perror("error: socket binding failed, aborting...");
            close(server_fd);
            return EXIT_FAILURE;
        }
    }
    printf("server listening on %s\n", SERVER_SOCKET_FILE);

    //* Listen for incoming connections
    //- The listen() syscall listens for incoming connections on the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, 5, specifies the maximum number of pending connections that can be queued.
    //? If the listen() syscall fails, it returns -1.
    if (listen(server_fd, BACKLOG) == -1) {
        perror("error: socket listening failed, aborting...");
        close(server_fd);
        return EXIT_FAILURE;
    }

    //* while loop to accept and handle connections
    while (1) {
        //* Accept incoming connection
        //- The accept() syscall accepts an incoming connection on the server socket.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, NULL, specifies the address of the client socket.
        //- The 3rd argument, NULL, specifies the size of the client socket address.
        //? If the accept() syscall fails, it returns -1.
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &(socklen_t){sizeof client_addr})) == -1) {
            perror("error: connection accepting failed, aborting...");
            return EXIT_FAILURE;
        }

        //* Receive messages from the client
        //- The read() syscall receives messages from the client.
        //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //? If the read() syscall fails, it returns -1.
        while ((bytes_received = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
            buffer[bytes_received] = '\0';  //- Add a null terminator to the end of the buffer
            printf("received message (%4ld byte): %s\n", bytes_received, buffer);

            //* Write messages to the client
            //- The write() syscall sends messages to the client.
            //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
            //- The 2nd argument, buffer, specifies the buffer that contains the message to be sent.
            //- The 3rd argument, strlen(buffer), specifies the size of the message.
            //? If the write() syscall fails, it returns -1.
            if (write(client_fd, buffer, strlen(buffer)) == -1) {
                perror("error: message sending failed, aborting...");
                close(client_fd);
                continue;
            }
            printf("   reply message (%4ld byte): %s\n", bytes_received, buffer);
        }

        //* Close the client socket
        close(client_fd);
    }

    //* Close the server socket
    close(server_fd);            //- Close the server socket
    unlink(SERVER_SOCKET_FILE);  //- Remove the server socket file
    return EXIT_SUCCESS;
}

//* Signal handler function
void sig_handler(int sig) {
    switch (sig) {
        case SIGABRT:
        case SIGINT:
        case SIGKILL:
        case SIGTERM:
            printf("\nSignal received, exiting...\n");
            unlink(SERVER_SOCKET_FILE);  //- Remove the server socket file
        default:
            break;
    }
}
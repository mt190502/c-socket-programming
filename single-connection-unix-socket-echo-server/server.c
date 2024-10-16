#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define BACKLOG 3                                   //- Maximum number of pending connections (if linux, you can use SOMAXCONN)
#define BUFFER_SIZE 1024                            //- Message buffer size
#define SERVER_SOCKET_FILE "/tmp/echo_server.sock"  //- Server socket file path

int main(void) {
    int server_fd, client_fd;        //- Define a file descriptor for the server socket
    struct sockaddr_un server_addr;  //- Define a struct for the server address
    char buffer[BUFFER_SIZE];        //- Define a buffer to store the received message
    ssize_t numbytes;                //- Define a variable to store the size of the received message

    //* Create a socket for the server
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, AF_UNIX, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for stream-oriented sockets.
    //- The 3rd argument, 0, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("error: socket creation failed, aborting...");
        return EXIT_FAILURE;
    }

    //* Set the server address
    //- The memset() function fills the server_addr struct with zeros.
    //- The sun_family field will be set to AF_UNIX, which specifies the address family of the socket.
    //- The sun_path field will be set to SERVER_SOCKET_FILE, which specifies the path of the server socket.
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_FILE, sizeof server_addr.sun_path - 1);

    unlink(SERVER_SOCKET_FILE);  //- Remove the existing server socket file

    //* Bind the server address to the server socket
    //- The bind() syscall binds the server address to the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, (struct sockaddr*)&server_addr, specifies the server address.
    //- The 3rd argument, sizeof(server_addr), specifies the size of the server address.
    //? If the bind() syscall fails, it returns -1.
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof server_addr) == -1) {
        perror("error: socket binding failed, aborting...");
        close(server_fd);
        return EXIT_FAILURE;
    }

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

    printf("server listening on %s\n", SERVER_SOCKET_FILE);

    //* Accept incoming connections
    //- The accept() syscall accepts an incoming connection on the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, NULL, specifies the address of the client socket.
    //- The 3rd argument, NULL, specifies the size of the client socket address.
    //? If the accept() syscall fails, it returns -1.
    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("error: connection accepting failed, aborting...");
        return EXIT_FAILURE;
    }

    //* while loop to accept and handle connections
    while (1) {
        //* Read messages from the client
        //- The read() syscall receives messages from the client.
        //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //? If the read() syscall fails, it returns -1.
        if ((numbytes = read(client_fd, buffer, BUFFER_SIZE)) == -1) {
            perror("error: message receiving failed, aborting...");
            close(client_fd);
            continue;
        }

        buffer[strcspn(buffer, "\n")] = 0;  //- Remove the newline character from the end of the buffer
        buffer[numbytes] = '\0';            //- Add a null terminator to the end of the buffer
        if (buffer[0] == '\0')              //- If the buffer is empty, skip the message
            continue;
        printf("received message (%4ld byte): %s\n", numbytes, buffer);

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
        printf("   reply message (%4ld byte): %s\n", numbytes, buffer);
    }

    //* Close the server socket
    close(client_fd);            //- Close the client socket
    close(server_fd);            //- Close the server socket
    unlink(SERVER_SOCKET_FILE);  //- Remove the server socket file
    return 0;
}
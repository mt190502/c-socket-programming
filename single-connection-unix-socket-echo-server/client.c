#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define BUFFER_SIZE 1024                            //- Message buffer size
#define SERVER_SOCKET_FILE "/tmp/echo_server.sock"  //- Server socket file path

int main(void) {
    int sock_fd;                     //- Define a file descriptor for the server socket
    struct sockaddr_un server_addr;  //- Define a struct for the server address
    char buffer[BUFFER_SIZE];        //- Define a buffer to store the received message
    ssize_t bytes_received;          //- Define a variable to store the size of the received message

    //* Create a socket for the server
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, AF_UNIX, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for stream-oriented sockets.
    //- The 3rd argument, 0, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
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

    //* Connect to the server
    //- The connect() syscall connects the client socket to the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the client socket.
    //- The 2nd argument, (struct sockaddr*)&server_addr, specifies the server address.
    //- The 3rd argument, sizeof(server_addr), specifies the size of the server address.
    //? If the connect() syscall fails, it returns -1.
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof server_addr) == -1) {
        if (errno == ENOENT) {
            printf("error: server socket file '%s' not found, aborting...", SERVER_SOCKET_FILE);
            close(sock_fd);
            return EXIT_FAILURE;
        } else {
            perror("error: socket connection failed, aborting...");
            close(sock_fd);
            return EXIT_FAILURE;
        }
    }

    //* while loop to send and receive messages
    while (1) {
        printf("client> ");

        //* read input from the stdin
        //- The fgets() function reads a line from the stdin and stores it in the buffer.
        //- The 1st argument, buffer, specifies the buffer to store the input.
        //- The 2nd argument, BUFFER_SIZE, specifies the size of the buffer.
        //- The 3rd argument, stdin, specifies the input stream.
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (feof(stdin)) {  //- Check if the end of the file has been reached
                printf("EOF");
                close(sock_fd);
                return EXIT_SUCCESS;
            }
            perror("error: reading from stdin failed, aborting...");
            return EXIT_FAILURE;
        }
        buffer[strcspn(buffer, "\n")] = '\0';  //- Remove the newline character from the buffer
        if (strlen(buffer) == 0)
            continue;  //- Skip empty messages

        //* Write messages to the server
        //- The write() syscall sends messages to the server.
        //- The 1st argument, server_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer that contains the message to be sent.
        //- The 3rd argument, strlen(buffer), specifies the size of the message.
        //? If the write() syscall fails, it returns -1.
        if (write(sock_fd, buffer, strlen(buffer)) == -1) {
            perror("error: message sending failed, aborting...");
            close(sock_fd);
            return EXIT_FAILURE;
        }

        //* Read messages from the server
        //- The read() syscall receives messages from the server.
        //- The 1st argument, server_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //- The 3rd argument, BUFFER_SIZE, specifies the size of the buffer.
        //? If the read() syscall fails, it returns -1.
        if ((bytes_received = read(sock_fd, buffer, BUFFER_SIZE)) > 0) {
            buffer[bytes_received] = '\0';  //- Add a null terminator to the end of the buffer
            printf("server> %s\n", buffer);
        }
    }

    //* Close the client socket
    close(sock_fd);
    return EXIT_SUCCESS;
}
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024       //- Message buffer size
#define SERVER_IP "127.0.0.1"  //- Server IP address
#define SERVER_PORT 8080       //- Server port number

int main(void) {
    int client_fd;                   //- Define a file descriptor for the client socket
    struct sockaddr_in server_addr;  //- Define a struct for the server address
    char buffer[BUFFER_SIZE];        //- Define a buffer to store the received message
    int bytes_received;              //- Define a variable to store the size of the received message

    //* Create a socket for the client
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, PF_INET, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for TCP sockets.
    //- The 3rd argument, IPPROTO_TCP, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((client_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return 1;
    }

    //* Set the dest server address
    //- The sin_addr.s_addr field of the server_addr struct is set to the IP address of the server.
    //- The sin_port field of the server_addr struct is set to the port number of the server.
    //- The sin_family field will be set to PF_INET, which specifies the address family of the socket.
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = PF_INET;

    //* Connect to the server
    //- The connect() syscall connects the client socket to the server socket.
    //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
    //- The 2nd argument, (struct sockaddr*)&server_addr, specifies the server address.
    //- The 3rd argument, sizeof(server_addr), specifies the size of the server address.
    //? If the connect() syscall fails, it returns -1.
    if ((connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
        perror("error: socket connection failed, aborting...");
        return 1;
    }

    //* while loop to send and receive messages from the server
    while (1) {
        printf("client> ");  //- Print the client prompt

        //* read input from the stdin
        //- The fgets() function reads a line from the stdin and stores it in the buffer.
        //- The 1st argument, buffer, specifies the buffer to store the input.
        //- The 2nd argument, BUFFER_SIZE, specifies the size of the buffer.
        //- The 3rd argument, stdin, specifies the input stream.
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (feof(stdin)) {  //- Check if the end of the file has been reached
                printf("EOF\n");
                break;
            }
            perror("error: reading from stdin failed, aborting...");
            return 1;
        }

        buffer[strcspn(buffer, "\n")] = '\0';  //- Remove the newline character from the buffer
        if (strlen(buffer) == 0)
            continue;

        //* Send the message to the server
        //- The send() syscall sends the message to the server socket.
        //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the message to be sent.
        //- The 3rd argument, strlen(buffer), specifies the size of the message.
        //? If the send() syscall fails, it returns -1.
        if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
            perror("error: socket sending failed, aborting...");
            return 1;
        }

        //* Receive messages from the server
        //- The recv() syscall receives messages from the server.
        //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //- The 3rd argument, BUFFER_SIZE, specifies the size of the buffer.
        //- The 4th argument, 0, specifies the flags. 0 is the standard mode for the recv() syscall.
        //? If the recv() syscall fails, it returns -1.
        if ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("server> %s\n", buffer);
        }

        //* Close the client socket
        close(client_fd);
    }

    return 0;
}

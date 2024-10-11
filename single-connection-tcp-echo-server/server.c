#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 1              //- If the server is busy, it will allow up to 3 pending connections
#define BUFFER_SIZE 1024       //- Message buffer size
#define SERVER_IP "127.0.0.1"  //- Server IP address
#define SERVER_PORT 8080       //- Server sport number

int main(void) {
    int server_fd, client_fd;                     //- Define file descriptors for the server and client sockets
    struct sockaddr_in source_addr, client_addr;  //- Define structs for the server and client addresses
    socklen_t client_addr_len;                    //- Define the length of the client address
    char buffer[BUFFER_SIZE];                     //- Define a buffer to store the received message
    int bytes_received;                           //- Define a variable to store the size of the received message

    //* Create a socket for the server
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, PF_INET, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for TCP sockets.
    //- The 3rd argument, IPPROTO_TCP, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return 1;
    }

    //* Set the socket option
    //- The setsockopt() syscall sets the socket options for the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, SOL_SOCKET, specifies the level at which the option is defined.
    //- The 3rd argument, SO_REUSEADDR, specifies the option to be set.
    //- The 4th argument, &(int){1}, specifies the value of the option.
    //- The 5th argument, sizeof(int), specifies the size of the option value.
    //? If the setsockopt() syscall fails, it returns -1.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("error: socket option failed, aborting...");
        return 1;
    }

    //* Set the source server address
    //- The sin_addr.s_addr field of the source_addr struct is set to the IP address of the server.
    //- The sin_port field of the source_addr struct is set to the port number of the server.
    //- The sin_family field will be set to PF_INET, which specifies the address family of the socket.
    source_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    source_addr.sin_port = htons(SERVER_PORT);
    source_addr.sin_family = PF_INET;

    //* Bind the server address to the server socket
    //- The bind() syscall binds the server address to the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, (struct sockaddr*)&source_addr, specifies the server address.
    //- The 3rd argument, sizeof(source_addr), specifies the size of the server address.
    //? If the bind() syscall fails, it returns -1.
    if ((bind(server_fd, (struct sockaddr*)&source_addr, sizeof(source_addr))) == -1) {
        perror("error: socket binding failed, aborting...");
        return 1;
    }
    printf("server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    //* while loop to listen for incoming connections
    while (1) {
        //* Listen for incoming connections
        //- The listen() syscall listens for incoming connections on the server socket.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, BACKLOG, specifies the maximum number of pending connections that the server will allow.
        //? If the listen() syscall fails, it returns -1.
        if ((listen(server_fd, BACKLOG)) == -1) {
            perror("error: socket listening failed, aborting...");
            return 1;
        }

        //* Accept incoming connections
        //- client_addr_len is required to store the size of the client address.
        //- The accept() syscall accepts an incoming connection on the server socket.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, (struct sockaddr*)&client_addr, specifies the client address.
        //- The 3rd argument, &client_addr_len, specifies the size of the client address.
        //? If the accept() syscall fails, it returns -1.
        client_addr_len = sizeof(client_addr);
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
            perror("error: socket accepting failed, aborting...");
            return 1;
        }

        //* Receive messages from the client
        //- The recv() syscall receives messages from the client.
        //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //- The 3rd argument, BUFFER_SIZE, specifies the size of the buffer.
        //- The 4th argument, specifies the flags. 0 is standard mode for recv() syscall.
        while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';  //- Add a null terminator to the end of the message.

            //- Print the received message from the client.
            printf("received message from %s:%d (%4d byte): %s\n", inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port), bytes_received, buffer);

            //* Echo the message back to the client
            //- The send() syscall sends the message back to the client.
            //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
            //- The 2nd argument, buffer, specifies the buffer containing the message.
            //- The 3rd argument, strlen(buffer), specifies the size of the message.
            //- The 4th argument, 0, specifies the flags. 0 is standard mode for send() syscall.
            if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                perror("error: socket sending failed, aborting...");
                return 1;
            } else {
                printf("     reply message to %s:%d (%4d byte): %s\n", inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port), bytes_received, buffer);
            }
        }

        //* Close the client socket
        close(client_fd);
    }

    return 0;
}
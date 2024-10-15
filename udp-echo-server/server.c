#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024       //- Buffer size
#define SERVER_IP "127.0.0.1"  //- Server ip address
#define SERVER_PORT 8080       //- Server port number

int main(void) {
    int server_fd;                                //- Define a file descriptor for the server socket
    struct sockaddr_in server_addr, client_addr;  //- Define structs for the server and client addresses
    char buffer[BUFFER_SIZE];                     //- Define a buffer to store the received message
    socklen_t addr_len;                           //- Define the length of the client address
    ssize_t numbytes;                             //- Define a variable to store the size of the received message

    //* Create a socket for the server
    //- The socket() syscall creates a new socket and returns a file descriptor that refers to that socket.
    //- The 1st argument, PF_INET, specifies the address family of the socket.
    //- The 2nd argument, SOCK_DGRAM, specifies the type of the socket. SOCK_DGRAM is used for UDP sockets.
    //- The 3rd argument, IPPROTO_UDP, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((server_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return EXIT_FAILURE;
    }

    //* Set the server address
    //- The memset() function fills the server_addr struct with zeros.
    //- The sin_addr.s_addr field of the server_addr struct is set to the IP address of the server.
    //- The sin_port field of the server_addr struct is set to the port number of the server.
    //- The sin_family field will be set to PF_INET, which specifies the address family of the socket.
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = PF_INET;                    // IPv4
    server_addr.sin_port = htons(SERVER_PORT);           // port number
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // Host address

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
    printf("server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    //* while loop to receive and send messages
    while (1) {
        addr_len = sizeof(client_addr);  //- addr_len is required to store the size of the client address

        //* Receive messages from the client
        //- The recvfrom() syscall receives messages from the client.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, buffer, specifies the buffer to store the received message.
        //- The 3rd argument, BUFFER_SIZE, specifies the size of the buffer.
        //- The 4th argument, 0, specifies the flags.
        //- The 5th argument, (struct sockaddr*)&client_addr, specifies the client address.
        //- The 6th argument, &addr_len, specifies the size of the client address.
        //? If the recvfrom() syscall fails, it returns -1.
        if ((numbytes = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &addr_len)) == -1) {
            perror("error: message receiving failed, aborting...");
            continue;
        }
        buffer[numbytes] = '\0';  //- Add a null terminator to the end of the buffer
        printf("received message from %s:%d (%4ld byte): %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), numbytes, buffer);

        //* Send the message back to the client
        //- The sendto() syscall sends the message back to the client.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, buffer, specifies the buffer to send.
        //- The 3rd argument, numbytes, specifies the size of the buffer.
        //- The 4th argument, 0, specifies the flags.
        //- The 5th argument, (struct sockaddr*)&client_addr, specifies the client address.
        //- The 6th argument, addr_len, specifies the size of the client address.
        //? If the sendto() syscall fails, it returns -1.
        if (sendto(server_fd, buffer, numbytes, 0, (struct sockaddr*)&client_addr, addr_len) == -1) {
            perror("sendto");
            continue;
        }
    }

    //* Close the server socket
    close(server_fd);
    return 0;
}

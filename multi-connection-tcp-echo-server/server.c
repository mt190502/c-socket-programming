#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define BACKLOG 3              //- If the server is busy, it will allow up to 3 pending connections (if linux, you can set it to SOMAXCONN)
#define BUFFER_SIZE 1024       //- Message buffer size
#define SERVER_IP "127.0.0.1"  //- Server IP address
#define SERVER_PORT 8080       //- Server sport number

void sig_handler(int sig);

int main(void) {
    int server_fd, client_fd;                     //- Define file descriptors for the server and client sockets
    struct sockaddr_in source_addr, client_addr;  //- Define structs for the server and client addresses
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
    //- The 1st argument, PF_INET, specifies the address family of the socket.
    //- The 2nd argument, SOCK_STREAM, specifies the type of the socket. SOCK_STREAM is used for TCP sockets.
    //- The 3rd argument, IPPROTO_TCP, specifies the protocol to be used with the socket.
    //? If the socket() syscall fails, it returns -1.
    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("error: socket creation failed, aborting...");
        return EXIT_FAILURE;
    }

    //* Set the socket option
    //- The setsockopt() syscall sets the socket options for the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, SOL_SOCKET, specifies the level at which the option is defined.
    //- The 3rd argument, SO_REUSEADDR, specifies the option to be set.
    //- The 4th argument, &(int){1}, specifies the value of the option. (1 is true)
    //- The 5th argument, sizeof(int), specifies the size of the option value.
    //? If the setsockopt() syscall fails, it returns -1.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("error: socket option failed, aborting...");
        return EXIT_FAILURE;
    }

    //* Set the source server address
    //- The memset() function fills the source_addr struct with zeros.
    //- The sin_addr.s_addr field of the source_addr struct is set to the IP address of the server.
    //- The sin_port field of the source_addr struct is set to the port number of the server.
    //- The sin_family field will be set to PF_INET, which specifies the address family of the socket.
    memset(&source_addr, 0, sizeof source_addr);
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
    printf("server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    //* Listen for incoming connections
    //- The listen() syscall listens for incoming connections on the server socket.
    //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
    //- The 2nd argument, BACKLOG, specifies the maximum number of pending connections that the server will allow.
    //? If the listen() syscall fails, it returns -1.
    if ((listen(server_fd, BACKLOG)) == -1) {
        perror("error: socket listening failed, aborting...");
        close(server_fd);
        return EXIT_FAILURE;
    }

    //* while loop to listen for incoming connections
    while (1) {
        //* Accept incoming connections
        //- client_addr_len is required to store the size of the client address.
        //- The accept() syscall accepts an incoming connection on the server socket.
        //- The 1st argument, server_fd, specifies the file descriptor of the server socket.
        //- The 2nd argument, (struct sockaddr*)&client_addr, specifies the client address.
        //- The 3rd argument, &client_addr_len, specifies the size of the client address.
        //? If the accept() syscall fails, it returns -1.
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &(socklen_t){sizeof(client_addr)})) == -1) {
            perror("error: socket accepting failed, aborting...");
            return EXIT_FAILURE;
        }

        //* Fork the process to handle multiple connections
        pid_t pid = fork();

        //* Handle the child process
        //- If the pid is 0, the process is the child process. If the pid is -1, the fork() syscall failed.
        //- The child process will echo the message back to the client.
        if (pid == -1) {
            perror("error: fork failed, aborting...");
            close(client_fd);
            return EXIT_FAILURE;
        } else if (pid == 0) {  //- In the child process
            //* Close the server socket in the child process
            //- If the server socket is not closed in the child process, the server will not be able to accept new connections.
            close(server_fd);

            //* Receive messages from the client
            //- The recv() syscall receives messages from the client.
            //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
            //- The 2nd argument, buffer, specifies the buffer to store the received message.
            //- The 3rd argument, BUFFER_SIZE, specifies the size of the buffer.
            //- The 4th argument, specifies the flags. 0 is standard mode for recv() syscall.
            printf("  new connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
                buffer[bytes_received] = '\0';  //- Add a null terminator to the end of the message.
                printf("received message from %s:%d (%4ld byte): %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), bytes_received,
                       buffer);

                //* Echo the message back to the client
                //- The send() syscall sends the message back to the client.
                //- The 1st argument, client_fd, specifies the file descriptor of the client socket.
                //- The 2nd argument, buffer, specifies the buffer containing the message.
                //- The 3rd argument, strlen(buffer), specifies the size of the message.
                //- The 4th argument, 0, specifies the flags. 0 is standard mode for send() syscall.
                if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                    perror("error: socket sending failed, aborting...");
                    close(client_fd);
                    return EXIT_FAILURE;
                }
                printf("     reply message to %s:%d (%4ld byte): %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), bytes_received,
                       buffer);
            }

            //- If the bytes_received is 0, the client disconnected.
            if (bytes_received == 0) {
                printf("client %s:%d disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            } else {
                perror("error: socket receiving failed, aborting...");
            }

            //* Close the client socket
            close(client_fd);
            return EXIT_SUCCESS;
        }

        close(client_fd);
    }

    //* Close the client socket
    close(client_fd);
    return EXIT_SUCCESS;
}

void sig_handler(int sig) {
    switch (sig) {
        case SIGABRT:
        case SIGINT:
        case SIGKILL:
        case SIGTERM:
            printf("signal %d received, exiting...\n", sig);
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}
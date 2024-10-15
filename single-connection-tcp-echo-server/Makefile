CC = gcc

# Build server and client
all: server client

# Server build rule
server: server.c
	$(CC) -o server server.c

# Client build rule
client: client.c
	$(CC) -o client client.c

# Clean up compiled files
clean:
	rm -f server client

.PHONY: all clean

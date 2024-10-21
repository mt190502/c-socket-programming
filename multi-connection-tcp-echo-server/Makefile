CC = gcc
CFLAGS = -ggdb3 -O0 -Wall -Wextra -Wpedantic -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=undefined

# Build server and client
all: server client

# Server build rule
server: server.c
	$(CC) $(CFLAGS) -o server server.c

# Client build rule
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Clean up compiled files
clean:
	rm -f server client

.PHONY: all clean

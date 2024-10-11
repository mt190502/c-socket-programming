# Single Connection TCP Echo Server

This is a simple TCP echo server that listens on a specified port and echos back any data it receives.

## Usage

To run the server, follow these steps:

1. Clone the repository:

    ```bash
    git clone <this repo>
    ```

2. Change into the project directory:

    ```bash
    cd single-connection-tcp-echo-server
    ```

3. Build server and client:

    ```bash
    make
    ```

4. Run the server a terminal:

    ```bash
    ./server
    ```

5. In another terminal, run the client:

    ```bash
    ./client
    ```

6. Type a message in the client terminal and press enter. The server will echo the message back to the client.

# UDP Echo Server

This is a simple UDP echo server that listens on a specified port and echos back any data it receives.

## Usage

To run the server, follow these steps:

1. Clone the repository:

    ```bash
    git clone <this repo>
    ```

2. Change into the project directory:

    ```bash
    cd udp-echo-server
    ```

3. Build server and client:

    ```bash
    make
    ```

4. Run the server a terminal:

    ```bash
    ./server
    ```

5. In another terminal, run the client(s):

    Terminal 1:

    ```bash
    ./client
    ```

    Terminal 2:

    ```bash
    ./client
    ```

6. Type a message in one of the client terminals and press enter. The server will echo the message back to the client.
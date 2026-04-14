#include "proxy.h"
#include "thread_pool.h"

// import libraries
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h> // networking operations
#include <unistd.h>    // to allow for low-level calls

// define stuct to allow multiple backends

int main(int argc, char *argv[])
{

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket
    sockaddr_in server_addr{};
    int port = atoi(argv[1]);

    // define multiple backends and store them in a vector
    std::vector<Backend> backends = {
        {"127.0.0.1", 9001},
        {"127.0.0.1", 9002},
        {"127.0.0.1", 9003}};

    // create thread pool
    start_thread_pool(4);
    sleep(1);

    if (listen_fd < 0)
    {
        // check that socket has been successfully created
        std::cerr << "Socket creation failed\n";
        return EXIT_FAILURE;
    }

    // fill address details
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // assign the address to the socket
    if (bind(listen_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Socket binding failed\n";
        return EXIT_FAILURE;
    }

    // make socket_fd passive so it can listen for and
    // accept connections
    if (listen(listen_fd, 5) < 0)
    {
        std::cerr << "Listening error/ Connection refused\n";
        return EXIT_FAILURE;
    }

    std::cout << "Proxy listening on port " << port << ".\n";

    // Round Robin Counter
    int next_backend = 0;

    // infinite loop to continue accepting clients
    while (true)
    {
        // address of client
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // accept connection request from client and create client socket for the client
        // client address gets saved in client_addr
        int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);

        std::cout << "In main, client_fd= " << client_fd << "\n";

        if (client_fd < 0)
        {
            std::cerr << "Accept failed.\n";
            continue;
        }

        // get next backend
        Backend backend = backends[next_backend % backends.size()];

        // queue the connection
        std::cout << "Calling enqueue_connection\n";
        enqueue_connection(client_fd, backend);

        // char buffer[1024] = {0};

        // // read message from client and write into buffer using recv
        // int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        // std::cout << "Recieved: " << buffer << "\n";

        // const std::string response = "Hello from server " + std::to_string(port) + "\n";

        // // send server response to client
        // send(client_socket, response.c_str(), response.length(), 0);
        // close(client_fd)

        next_backend++; // next backend
    }

    close(listen_fd); // close program's listener
}

#include "proxy.h"
#include "thread_pool.h"

// import libraries
#include <iostream>
#include <vector>
#include <memory>
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
    // need to use unique_ptr to prevent copying during
    // vector reallocation. Backend contains a member
    // of atomic type so cannot be copied
    std::vector<std::unique_ptr<Backend>> backends;
    backends.push_back(std::make_unique<Backend>("127", 9001));
    backends.push_back(std::make_unique<Backend>("127", 9002));
    backends.push_back(std::make_unique<Backend>("127", 9003));

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

    // make listen_fd passive so it can listen for and
    // accept connections
    if (listen(listen_fd, 5) < 0)
    {
        std::cerr << "Listening error/ Connection refused\n";
        return EXIT_FAILURE;
    }

    std::cout << "Proxy listening on port " << port << ".\n";

    // Round Robin Counter
    // int next_backend = 0;

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

        // This is by round robin
        // Backend backend = backends[next_backend % backends.size()];

        // This gets the next backend using least connections
        int index = 0;
        int min_connections = backends[0]->num_connections.load();
        for (int i = 1; i < backends.size(); i++)
        {
            int connections = backends[i]->num_connections.load();
            if (connections < min_connections)
            {
                min_connections = connections;
                int index = i;
            }
        }

        // queue the connection
        std::cout << "Calling enqueue_connection\n";
        enqueue_connection(client_fd, *backends[index]);

        // char buffer[1024] = {0};

        // next_backend++; // next backend
    }

    close(listen_fd); // close program's listener
}

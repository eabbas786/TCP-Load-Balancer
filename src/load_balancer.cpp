
// import libraries
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>    // to allow for low-level calls
#include <arpa/inet.h> // networking operations

#include "proxy.h"

int main(int argc, char *argv[])
{

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket
    sockaddr_in server_addr;
    int port = atoi(argv[1]);

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

    if (listen(listen_fd, 5) < 0)
    {
        std::cerr << "Listening error/ Connection refused\n";
        return EXIT_FAILURE;
    }

    std::cout << "Proxy listening on port " << port << ".\n";

    // infinite loop to continue accepting clients
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_len);

        if (client_fd < 0)
        {
            std::cerr << "Accept failed.\n";
            continue;
        }

        handle_client(client_fd);

        // char buffer[1024] = {0};

        // // read message from client and write into buffer using recv
        // int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        // std::cout << "Recieved: " << buffer << "\n";

        // const std::string response = "Hello from server " + std::to_string(port) + "\n";

        // // send server response to client
        // send(client_socket, response.c_str(), response.length(), 0);

        close(client_fd); // close client socket
    }

    close(listen_fd); // close program's listener
}

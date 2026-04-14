
// import libraries
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h> // networking operations
#include <unistd.h>    // to allow for low-level calls

int main(int argc, char *argv[])
{

    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket
    sockaddr_in server_addr{};
    int port = atoi(argv[1]);

    if (server_fd < 0)
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
    if (bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Socket binding failed\n";
        return EXIT_FAILURE;
    }

    // make listen_fd passive so it can listen for and
    // accept connections
    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "Listening error/ Connection refused\n";
        return EXIT_FAILURE;
    }

    std::cout << "Server listening on port " << std::to_string(port) << "\n";

    // infinite loop to continue accepting clients
    while (true)
    {
        // address of client
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // accept connection request from client and create client socket for the client
        // client address gets saved in client_addr
        int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);

        if (client_fd < 0)
        {
            std::cerr << "Accept failed.\n";
            continue;
        }

        char buffer[1024] = {0};

        // read message from client and write into buffer using recv
        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

        std::cout << "Recieved: " << buffer << "\n";

        const std::string response = "Hello from server " + std::to_string(port) + "\n";

        // send server response to client
        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
    }

    close(server_fd); // close program's listener
}

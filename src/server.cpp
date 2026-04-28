
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

        char buffer[4096]; // for storing data

        while (true)
        {
            // read size of received message
            uint32_t len;
            int bytes = 0;
            while (bytes < sizeof(len))
            {
                int n = recv(client_fd, (char *)&len + bytes, sizeof(len) - bytes, 0);
                if (n <= 0)
                    break;
                bytes += n;
            }

            if (bytes < sizeof(len))
                break; // did not read full message size

            uint32_t msg_siz = ntohl(len); // convert from network to host byte order
                                           // len: network byte order
                                           // msg_size: host byte order

            // get message sent from client
            bytes = 0;
            while (bytes < msg_siz)
            {
                int n = recv(client_fd, buffer + bytes, msg_siz - bytes, 0);
                if (n <= 0)
                    break;

                bytes += n;
            }

            if (bytes < msg_siz)
                break; // did not read full message

            std::cout << "Recieved: " << bytes << " bytes\n\n";
            std::cout.write(buffer, bytes);
            std::cout << "\n\n";

            const std::string response = "Hello from server " + std::to_string(port) + "\n";
            uint32_t response_len = htonl(response.size()); // convert size from host byte order to network byte order

            // send message size to client
            bytes = 0;
            while (bytes < sizeof(response_len))
            {
                int n = send(client_fd, (char *)&response_len + bytes, sizeof(response_len) - bytes, MSG_NOSIGNAL);
                if (n <= 0)
                    break;
                bytes += n;
            }

            if (bytes < sizeof(response_len))
                break; // failed to send message size

            // send server response to client
            int total = 0;
            while (total < response.size())
            {
                int n = send(client_fd, response.c_str() + total, response.size() - total, MSG_NOSIGNAL);
                if (n <= 0)
                    break;
                total += n;
            }

            if (total < response.size())
                break; // failed to send entire message
        }
        close(client_fd); // close client
    }
    close(server_fd); // close program's listener
}

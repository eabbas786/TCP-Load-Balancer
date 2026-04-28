
// import libraries
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h> // networking operations
#include <unistd.h>    // to allow for low-level calls

#include <algorithm> // for min

bool is_recv_all(int fd, char *buf, int len)
{
    // get size (in bytes) of sent message
    int total = 0;
    while (total < len)
    {

        int n = recv(fd, buf + total, len - total, 0);
        if (n <= 0)
            return false; // error handling
        total += n;
    }

    return true;
}

// this function sends the entire message to a connected socket (fd),
// while handling socket blocking and partial sends
bool is_send_all(int fd, const char *buf, int len)
{
    int total = 0;
    while (total < len)
    {

        int n = send(fd, buf + total, len - total, MSG_NOSIGNAL);
        if (n <= 0)
            return false; // error handling

        total += n;
    }
    return true;
}

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
            // get size (in bytes) of sent message
            uint32_t len;
            // get the length; break out if error
            if (!is_recv_all(client_fd, (char *)&len, sizeof(len)))
                break;

            uint32_t msg_siz = ntohl(len); // convert from network to host byte order
                                           // len: network byte order
                                           // msg_size: host byte order

            // get message sent from client
            size_t total = msg_siz;
            while (total > 0)
            {
                size_t chunk = std::min(total, sizeof(buffer));
                if (!is_recv_all(client_fd, buffer, chunk))
                    break;

                std::cout << "Recieved:\n\n";
                std::cout.write(buffer, chunk);
                std::cout << "\n\n";

                total -= chunk;
            }

            const std::string response = "Hello from server " + std::to_string(port) + "\n";
            uint32_t response_len = htonl(response.size()); // convert size from host byte order to network byte order

            // send message size to client
            if (!is_send_all(client_fd, (char *)&response_len, sizeof(response_len)))
                break;

            // send message to client
            if (!is_send_all(client_fd, response.c_str(), response.size()))
                break;
        }
        close(client_fd); // close client
    }
    close(server_fd); // close program's listener
}

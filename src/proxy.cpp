
#include "proxy.h"

#include <iostream>
#include <fcntl.h> // to stop blocking
#include <arpa/inet.h>
#include <unistd.h>

void handle_client(int client_fd, Backend backend)
{
    // set client_fd to non_blocking so thread does not get stuck
    int flags = fcntl(client_fd, F_GETFL, 0); // Get current flags
    if (flags == -1)
        return;
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK); // Add non-blocking flag

    std::cout << "1) IN handle_client\n";

    // create socket that will connect to backend server's listener
    int upstream_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (upstream_fd < 0)
    {
        std::cout << "Upstream Socket creation failed\n";
        return;
    }

    std::cout << "2) Upstream Socket created!\n";

    // get address for backend server's listener
    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend.port);                       // port for backend server
    inet_pton(AF_INET, (char *)&backend.host, &backend_addr.sin_addr); // converts TCP address into its numeric binary form

    // connect backend socket to backend's listener,
    // where backend_addr is associated with the backend server's listener
    std::cout << "Connecting to "
              << backend.host << ":"
              << backend.port << std::endl;
    if (connect(upstream_fd, (sockaddr *)&backend_addr, sizeof(backend_addr)) < 0)
    {
        std::cerr << "connect failed: Connection refused";
        close(upstream_fd);
        return;
    }

    std::cout << "3) Connected to backend\n";

    char buffer[4096]; // buffer to read and store sent data

    while (true)
    {

        // read bytes sent by client to proxy into buffer
        std::cout << "4) About to recieve from client " << client_fd << "\n";
        int bytes = recv(client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        // if (bytes <= 0)
        //     break;

        std::cout << "5) Recieve from client return " << bytes << "bytes \n";

        // send the collected data from proxy to backend server
        std::cout
            << "6) Forwarding to backend: "
            << backend.host
            << ":"
            << backend.port
            << std::endl;

        send(upstream_fd, buffer, bytes, 0);

        // read data sent from backend server to proxy
        std::cout << "7) About to recieve from Backend\n";
        bytes = recv(upstream_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        std::cout << "8) Recieved: " << buffer << " from Backend\n";

        // send data sent by server to client
        std::cout << "9) Sending to client\n";
        send(client_fd, buffer, bytes, 0);

        std::cout << "10) Done\n";
    }

    // close sockets
    close(upstream_fd);
}
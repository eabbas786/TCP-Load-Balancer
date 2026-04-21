
#include "proxy.h"

#include <thread>
#include <iostream>
#include <cstdio> //perror
#include <arpa/inet.h>
#include <unistd.h>

// helper function to handle sending/ recieving data
void handle_data(int src, int dest)
{
    char buffer[4096];
    while (true)
    {
        int bytes = recv(src, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        std::cout << "Recieved: " << bytes << " bytes\n\n";
        std::cout.write(buffer, bytes);
        std::cout << "\n\n";

        int total = 0;
        while (total < bytes)
        {
            int n = send(dest, buffer + total, bytes - total, MSG_NOSIGNAL);
            if (n <= 0)
                break;
            total += n;
        }
    }
}
void handle_client(int client_fd, Backend *backend)
{

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
    backend_addr.sin_port = htons(backend->port);                       // port for backend server
    inet_pton(AF_INET, (char *)&backend->host, &backend_addr.sin_addr); // converts TCP address into its numeric binary form

    // connect backend socket to backend's listener,
    // where backend_addr is associated with the backend server's listener
    std::cout << "Connecting to "
              << backend->host << ":"
              << backend->port << std::endl;
    if (connect(upstream_fd, (sockaddr *)&backend_addr, sizeof(backend_addr)) < 0)
    {

        std::perror("connect failed: Connection refused");
        close(upstream_fd);
        return;
    }

    std::cout << "3) Connected to backend\n";

    std::thread thread1(handle_data, client_fd, upstream_fd);
    std::thread thread2(handle_data, upstream_fd, client_fd);

    thread1.join(); // handle data sent from client to server
    thread2.join(); // handle data sent from server to client

    close(upstream_fd); // close socket

    // char buffer[4096]; // buffer to read and store sent data

    // while (true)
    // {

    // read bytes sent by client to proxy into buffer
    // std::cout << "4) About to recieve from client " << client_fd << "\n";

    // std::cout << "4) C"

    // std::cout << "5) Recieve from client return " << bytes << " bytes \n";
    // std::cout << "Recieved: ";
    // std::cout.write(buffer, bytes);
    // std::cout << "\nfrom Client\n ";

    // send the collected data from proxy to backend server
    // std::cout
    //     << "6) Forwarding to backend: "
    //     << backend->host
    //     << ":"
    //     << backend->port
    //     << std::endl;

    // int sent = 0;
    // while (sent < bytes)
    // {
    //     int n = send(upstream_fd, buffer + sent, bytes - sent, 0);
    //     if (n <= 0)
    //         break;
    //     sent += n;
    // }

    // // read data sent from backend server to proxy
    // std::cout << "7) About to recieve from Backend\n";
    // bytes = recv(upstream_fd, buffer, sizeof(buffer), 0);
    // if (bytes <= 0)
    //     break;

    // std::cout << "8) Recieved: " << bytes << " bytes\n";
    // std::cout.write(buffer, bytes);
    // std::cout << "\nfrom Backend\n ";
    // // send data sent by server to client

    // std::cout
    //     << "9) Sending to client\n";
    // sent = 0;
    // while (sent < bytes)
    // {
    //     int n = send(client_fd, buffer + sent, bytes - sent, 0);
    //     if (n <= 0)
    //         break;
    //     sent += n;
    // }

    //     std::cout << "10) Done\n";
    // }

    // close sockets
    // close(upstream_fd);
}
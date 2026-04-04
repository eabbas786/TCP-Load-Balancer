#include "proxy.h"
#include <arpa/inet.h>
#include <unistd.h>

void handle_client(int client_fd)
{
    // create socket that will connect to backend server's listener
    int upstream_fd = socket(AF_INET, SOCK_STREAM, 0);

    // get address for backend server's listener
    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(9001);                     // port for backend server
    inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr); // converts TCP address into its numeric binary form

    // connect backend socket to backend's listener,
    // where backend_addr is associated with the backend server's listener
    connect(upstream_fd, (sockaddr *)&backend_addr, sizeof(backend_addr));

    char buffer[4096]; // buffer to read and store sent data

    while (true)
    {

        // read bytes sent by client to proxy into buffer
        int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        // send the collected data from proxy to backend server
        send(upstream_fd, buffer, bytes, 0);

        // read data sent from backend server to proxy
        bytes = recv(upstream_fd, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        // send data sent by server to client
        send(client_fd, buffer, bytes, 0);
    }

    // close sockets
    close(client_fd);
    close(upstream_fd);
}
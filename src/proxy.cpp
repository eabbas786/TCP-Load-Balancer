
#include "proxy.h"

#include <atomic>
#include <thread>
#include <sys/select.h> // for select: prevent blocking
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstdio> //perror
#include <algorithm>

std::atomic_bool running = true; // for handling connection if one side closes

// this function receives the entire message from a connected socket (fd),
// while handling socket blocking and partial reads
int recv_all(int fd, char *buf, int len)
{
    // get size (in bytes) of sent message
    int total = 0;
    while (total < len)
    {

        int n = recv(fd, buf + total, len - total, 0);
        if (n <= 0)
            return -1; // error handling
        total += n;
    }

    return 1;
}

// this function sends the entire message to a connected socket (fd),
// while handling socket blocking and partial sends
int send_all(int fd, const char *buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int n = send(fd, buf + total, len - total, MSG_NOSIGNAL);
        if (n <= 0)
            return -1; // error handling

        total += n;
    }
    return 1;
}

// this function shuts down the connections and updates the atomic variable
// to inform other threads of execution that the connection has been closed
void end_connection(int s1, int s2)
{
    running = false;
    shutdown(s1, SHUT_RDWR);
    shutdown(s2, SHUT_RDWR);
}

// helper function to handle sending/ recieving data
void handle_data(int src, int dest)
{
    char buffer[4096];
    while (true)
    {
        // get size (in bytes) of sent message
        uint32_t len;
        // get the length; break out if error
        int status = recv_all(src, (char *)&len, sizeof(len));
        if (status < 0)
        {
            end_connection(src, dest);
            break;
        }
        // send message length to dest; break if fail
        status = send_all(dest, (char *)&len, sizeof(len));
        if (status < 0)
        {
            end_connection(src, dest);
            break;
        }

        uint32_t msg_siz = ntohl(len); // len is in network byte order and msg_size host byte

        size_t total = msg_siz;
        while (total > 0)
        {
            size_t chunk = std::min(total, sizeof(buffer));
            status = recv_all(src, buffer, chunk);
            if (status < 0)
            {
                end_connection(src, dest);
                break;
            }

            std::cout << "Recieved:\n\n";
            std::cout.write(buffer, chunk);
            std::cout << "\n\n";

            // send message to dest
            status = send_all(dest, buffer, chunk);
            if (status < 0)
            {
                end_connection(src, dest);
                break;
            }

            total -= chunk;
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
}

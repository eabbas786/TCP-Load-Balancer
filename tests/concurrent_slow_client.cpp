

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio> //perror

#include <arpa/inet.h> // networking operations
#include <unistd.h>    // to allow for low-level calls i.e. POSIX

void client_worker(int id)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // define server to connect to; this is the proxy (the main program)
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // test if can connect to proxy
    if (connect(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::perror("connection failed");
        return;
    }

    for (int i = 0; i < 50; i++)
    {
        std::string msg = "client " + std::to_string(id) + " msg " + std::to_string(i) + "\n";
        uint32_t len = htonl(msg.size()); // convert size from host byte order to network byte order

        std::cout << "Sending message: " << msg << " to proxy\n";

        // send message size in network byte order
        int bytes = 0;
        while (bytes < sizeof(len))
        {
            int n = send(socket_fd, (char *)&len + bytes, sizeof(len) - bytes, MSG_NOSIGNAL);
            if (n <= 0)
                break;
            bytes += n;
        }

        if (bytes < sizeof(len))
            break; // failed to send message len; end connection

        bytes = 0;
        // loop using msg.size() since host byte order; not len
        while (bytes < msg.size())
        {
            int n = send(socket_fd, msg.c_str() + bytes, msg.size() - bytes, MSG_NOSIGNAL);
            if (n <= 0)
                break;
            bytes += n;
        }

        if (bytes < msg.size())
            break; // failed to send entire message

        char buffer[4096];

        // get size (in bytes) of received message
        uint32_t response_len; // will store in network byte order
        bytes = 0;
        while (bytes < sizeof(response_len))
        {
            int n = recv(socket_fd, (char *)&response_len + bytes, sizeof(response_len) - bytes, 0);
            if (n <= 0)
                break;
            bytes += n;
        }

        if (bytes < sizeof(response_len))
            break; // did not recieve right message length

        uint32_t msg_siz = ntohl(response_len); // convert into host byte order

        // get meesage sent from client
        bytes = 0;
        while (bytes < msg_siz)
        {
            int n = recv(socket_fd, buffer + bytes, msg_siz - bytes, 0);
            if (n <= 0)
                break;

            bytes += n;
        }

        if (bytes < msg_siz)
            break; // failed to read full message

        std::cout << "Recieved: \n";
        std::cout.write(buffer, bytes);
        std::cout << "\nfrom backend\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // add delay in between sends
    }

    close(socket_fd);
}

int main()
{
    const int NUM_CLIENTS = 100;

    std::vector<std::thread> threads;

    for (int i = 1; i <= NUM_CLIENTS; i++)
    {
        threads.emplace_back(client_worker, i); // constructs thread(client_worker, i) in place
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Stress test complete\n";
}

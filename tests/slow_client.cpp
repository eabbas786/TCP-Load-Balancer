

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio> //perror

#include <arpa/inet.h> // networking operations
#include <unistd.h>    // to allow for low-level calls i.e. POSIX

int main()
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
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 50; i++)
    {

        std::string msg = "client msg " + std::to_string(i) + "\n";
        std::cout << "Sending message: " << msg << " to proxy\n";
        send(socket_fd, msg.c_str(), msg.size(), 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // add delay in between sends
    }

    close(socket_fd);
}

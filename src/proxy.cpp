#include "proxy.h"
#include <arpa/inet.h>
#include <unistd.h>

void handle_client(int client)
{
    int backend = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(9001);
    inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr);

    connect(backend, (sockaddr *)&backend_addr, sizeof(backend_addr));

    char buffer[4096];

    while (true)
    {
        int bytes = recv(client, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        send(backend, buffer, bytes, 0);

        bytes = recv(backend, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
            break;

        send(client, buffer, bytes, 0);
    }

    close(client);
    close(backend);
}
#pragma once

#include <string>
struct Backend
{
    std::string host;
    int port;
    std::atomic_int num_connections;

    Backend(std::string host, int port)
    {
        this->host = host;
        this->port = port;
    }
};
void handle_client(int client, Backend *backend);

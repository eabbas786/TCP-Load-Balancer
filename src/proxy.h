#pragma once

#include <string>
struct Backend
{
    std::string host;
    int port;
};
void handle_client(int client, Backend backend);

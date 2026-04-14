#pragma once

#include "proxy.h"

struct Task
{
    int client_fd;
    Backend backend;
};

void start_thread_pool(int num_threads);
void enqueue_connection(int client_fd, Backend backend);
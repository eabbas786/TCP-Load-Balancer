#pragma once

#include "proxy.h"

#include "atomic"
#include <thread>
struct Task
{
    int client_fd;
    Backend *backend;
};

void start_thread_pool(int num_threads = std::thread::hardware_concurrency());
void enqueue_connection(int client_fd, Backend &backend);
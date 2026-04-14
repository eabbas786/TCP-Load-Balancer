#include "thread_pool.h"
#include "proxy.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#include <unistd.h> // to allow for low-level calls

std::queue<int> connectionQueue;  // to queue clients
std::queue<Backend> backendQueue; // keep track of backend
std::queue<Task> taskQueue;       // keep track of tasks
std::mutex queueMutex;            // mutual exclusion
std::condition_variable cv;

// worker threads
void worker()
{

    // infinite loop to keep on handling client
    while (true)
    {
        Task task;

        {
            // lock the queue so data can be shared safely
            // the mutex will be automatically unlocked when
            // it goes out of scope
            std::cout << "In worker; locking queue\n";
            std::unique_lock<std::mutex> lock(queueMutex);

            // wait until there is a task to be executed i.e. there is at least one client
            // releases mutex while sleeping
            // re-locks mutex when wakes up (once condition is satisfied)
            std::cout << "Checking condition\n";
            std::cout << "ADDR taskQueue before wait: " << &taskQueue << std::endl;
            cv.wait(lock, []
                    { return !taskQueue.empty(); });

            // get next connection
            std::cout << "Condition fulfilled; continuing with execution\n";
            task = taskQueue.front();
            taskQueue.pop();
        }

        // call handle_client to forward to backend
        std::cout << "Thread "
                  << std::this_thread::get_id()
                  << "handling client "
                  << task.client_fd
                  << std::endl;

        handle_client(task.client_fd, task.backend);
        close(task.client_fd); // close client socket
    }
}

// function for creating thread_pool
// called by load_balancer.cpp
void start_thread_pool(int num_threads = std::thread::hardware_concurrency())
{
    for (int i = 0; i < num_threads; i++)
    {
        // calling detach to seperate thread from the main thread
        // this allows the main thread to continue while the
        // thread runs in the background since the thread runs
        // an infinite loop.
        // will edit this logic after so that the ThreadPool is a class
        // that joins threads for graceful shutdown
        std::thread(worker).detach();
    }
}

// this function queues connections
void enqueue_connection(int client_fd, Backend backend)
{
    std::cout << "In enqueue_connection\n";
    {
        // lock queue from being accessed by any other thread
        std::lock_guard<std::mutex> lock(queueMutex);

        std::cout << "ADDR taskQueue in enque: " << &taskQueue << std::endl;
        std::cout << "Recieved " << client_fd << " client_fd\n";

        // push client socket that wants to connect and backend to forward to
        Task task = {client_fd, backend};
        taskQueue.push(task);
    }

    // notify a waiting thread that connectionQueue is not empty
    std::cout << "Calling notify_one\n";
    cv.notify_one();
}

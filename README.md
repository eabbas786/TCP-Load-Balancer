# TCP Load Balancer
A networking load balancer written in C++ that functions as a TCP proxy. It implements Least Connections load balancing using atomic counters to track active connections per backend and select the least-loaded server at runtime. Round Robin was previously implemented and explored for comparison during development.

The system uses POSIX sockets to accept client connections and establish bidirectional forwarding between clients and backend servers. It supports concurrent clients through multithreaded connection handling using a detached worker thread pool created at startup.
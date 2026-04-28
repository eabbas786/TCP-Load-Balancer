# TCP Load Balancer
A networking load balancer written in C++ that functions as a TCP proxy. It implements Least Connections load balancing using atomic counters to track active connections per backend and select the least-loaded server at runtime. Round Robin was previously implemented and explored for comparison during development.

The system uses POSIX sockets to accept client connections and dispatch them to a worker thread pool created at startup. Each worker thread handles an individual client connection by establishing a backend connection and spawning two threads per connection to manage bidirectional forwarding between client and backend, enabling concurrent full-duplex communication.

The system was validated under concurrent multi-client stress testing with simulated slow clients and high-throughput message bursts to evaluate stability and correctness under load. Proper handling of partial reads/writes and controlled connection shutdown ensures reliable forwarding without deadlocks or data loss.

A length-prefixed protocol is used to frame messages over TCP, ensuring correct reconstruction of message boundaries despite TCP’s stream-oriented nature.
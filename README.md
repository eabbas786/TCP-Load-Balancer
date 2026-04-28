# TCP Load Balancer
A networking load balancer written in C++ that functions as a TCP proxy. It implements Least Connections load balancing using atomic counters to track active connections per backend and select the least-loaded server at runtime. Round Robin was previously implemented and explored for comparison during development.

The system uses POSIX sockets to accept client connections and dispatch them to a worker thread pool created at startup. Each worker thread handles an individual client connection by establishing a backend connection and spawning two threads per connection to manage bidirectional forwarding between client and backend, enabling concurrent full-duplex communication.

To ensure correctness over TCP’s stream-oriented model, a length-prefixed protocol is used to preserve message boundaries. Messages are forwarded using chunked streaming, where large payloads are transmitted in fixed-size buffers while maintaining a single logical message frame, allowing efficient handling of arbitrarily large messages without excessive memory usage.

The system was validated through concurrent multi-client stress testing, including slow clients and large payload (multi-megabyte) transfers, ensuring reliable handling of partial reads/writes and proper connection shutdown without deadlocks or data loss.
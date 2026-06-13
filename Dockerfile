# Stage 1: Build stage
FROM gcc:14 AS builder

# Set the working directory
WORKDIR /app

# Copy directory into container
COPY . .

# Compile the code statically using Makefile
RUN make

#Stage 2: Runtime stage
FROM debian:trixie-slim

WORKDIR /app

# Copy static binary from builder stage
COPY --from=builder /app/build/load_balancer .
COPY --from=builder /app/build/server .

# Command to run binary
CMD ["./load_balancer", "8080"]


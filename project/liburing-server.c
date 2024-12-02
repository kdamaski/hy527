#include <liburing.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9999
#define BACKLOG 128
#define BUFFER_SIZE 1024

struct connection {
  int fd;
  char buffer[BUFFER_SIZE];
};

void fatal_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main() {
  int server_fd, client_fd;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  struct io_uring ring;
  struct io_uring_cqe *cqe;
  struct connection *conn;

  // Initialize server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    fatal_error("socket");
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    fatal_error("setsockopt");

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    fatal_error("bind");
  if (listen(server_fd, BACKLOG) < 0)
    fatal_error("listen");

  // Initialize io_uring
  if (io_uring_queue_init(32, &ring, 0) < 0)
    fatal_error("io_uring_queue_init");

  printf("Server listening on port %d\n", PORT);

  // Accept new connections
  while (1) {
    // Wait for an incoming connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen)) <
        0) {
      perror("accept");
      continue;
    }

    // printf("Accepted new connection: fd=%d\n", client_fd);

    // Allocate a connection structure
    conn = malloc(sizeof(*conn));
    conn->fd = client_fd;

    // Submit a read request
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recv(sqe, client_fd, conn->buffer, BUFFER_SIZE, 0);
    io_uring_sqe_set_data(sqe, conn);
    io_uring_submit(&ring);

    // Wait for the read to complete
    if (io_uring_wait_cqe(&ring, &cqe) < 0)
      fatal_error("io_uring_wait_cqe");

    // Process the read
    conn = io_uring_cqe_get_data(cqe);
    int bytes_read = cqe->res;
    if (bytes_read <= 0) {
      // printf("Client disconnected: fd=%d\n", conn->fd);
      close(conn->fd);
      free(conn);
    } else {
      // Echo the data back to the client
      // printf("Received data: %.*s", bytes_read, conn->buffer);

      const char *response = "HTTP/1.1 200 OK\r\n"
                             "Content-Length: 13\r\n"
                             "Connection: close\r\n"
                             "\r\n"
                             "Hello, World!";

      struct io_uring_sqe *write_sqe = io_uring_get_sqe(&ring);
      io_uring_prep_send(write_sqe, conn->fd, response, bytes_read, 0);
      io_uring_sqe_set_data(write_sqe, conn);
      io_uring_submit(&ring);
    }

    // Mark the completion queue entry as handled
    io_uring_cqe_seen(&ring, cqe);
  }

  close(server_fd);
  io_uring_queue_exit(&ring);
  return 0;
}


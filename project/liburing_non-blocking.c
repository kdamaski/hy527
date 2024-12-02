#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_DEPTH 1
#define BUFFER_SIZE 1024

int main() {
  struct io_uring ring;
  if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    return EXIT_FAILURE;
  }

  int fd = open("example.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    io_uring_queue_exit(&ring);
    return EXIT_FAILURE;
  }

  char *buffer = malloc(BUFFER_SIZE);
  struct iovec iov = {
      .iov_base = buffer,
      .iov_len = BUFFER_SIZE,
  };

  struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
  io_uring_prep_readv(sqe, fd, &iov, 1, 0);
  io_uring_submit(&ring);

  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&ring, &cqe);

  if (cqe->res > 0) {
    printf("Read %d bytes: %.*s\n", cqe->res, cqe->res, buffer);
  } else if (cqe->res == 0) {
    printf("EOF reached\n");
  } else {
    fprintf(stderr, "Error reading file: %s\n", strerror(cqe->res));
  }

  io_uring_cqe_seen(&ring, cqe);
  close(fd);
  free(buffer);
  io_uring_queue_exit(&ring);
  return EXIT_SUCCESS;
}

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#define BUFSZ 1024

// Set a file descriptor to non-blocking mode
void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL");
    exit(EXIT_FAILURE);
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    exit(EXIT_FAILURE);
  }
}

int main() {
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }

  // Open a file (or socket) and set it to non-blocking mode
  int file_fd = open("example.txt", O_RDONLY);
  if (file_fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  set_nonblocking(file_fd);

  // Add the file descriptor to the epoll instance
  struct epoll_event event;
  event.events = EPOLLIN;  // Monitor for input readiness
  event.data.fd = file_fd; // Attach the file descriptor to the event
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, file_fd, &event) == -1) {
    perror("epoll_ctl");
    close(file_fd);
    exit(EXIT_FAILURE);
  }

  // Event loop
  struct epoll_event events[10]; // Buffer for incoming events
  while (1) {
    int n = epoll_wait(epoll_fd, events, 10, 80); // Block for 80 ms or event
    if (n == -1) {
      perror("epoll_wait");
      break;
    }

    for (int i = 0; i < n; i++) {
      if (events[i].events & EPOLLIN) {
        // FD is ready for reading
        char buffer[BUFSZ];
        ssize_t bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
          printf("Read %zd bytes: %.*s\n", bytes_read, (int)bytes_read, buffer);
        } else if (bytes_read == 0) {
          printf("EOF reached\n");
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          close(events[i].data.fd);
        } else if (bytes_read == -1 && errno != EAGAIN) {
          perror("read");
        }
      }
    }
  }

  close(epoll_fd);
  return 0;
}

#include "thread-server.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9999

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  if (fcntl(server_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC) == -1) {
    perror("fcntl: set non-blocking");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(PORT),
                                    .sin_addr.s_addr = INADDR_ANY};
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 16) == -1) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1 failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev = {.events = EPOLLIN, .data.fd = server_fd};
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
    perror("epoll_ctl: add server_fd");
    close(server_fd);
    close(epoll_fd);
    exit(EXIT_FAILURE);
  }

  // Main event loop
  struct epoll_event events[MAX_EVENTS];

  printf("Server listening on port %d\n", PORT);
  while (1) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("epoll_wait failed");
      continue; // Retry on failure
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == server_fd) {
        // Accept new connection
        int client_fd = accept(server_fd, NULL, NULL);
        fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
      } else {
        int client_fd = events[i].data.fd;
        if (events[i].events & EPOLLIN) {
          char buf[1024];
          int n = recv(client_fd, buf, sizeof(buf), 0);
          if (n > 0) {
            buf[n] = '\0'; // Null-terminate the received string
            if (strstr(buf, "GET /large_file") != NULL) {
              // Open the requested file
              int file_fd =
                  open("./largefile0", O_NONBLOCK | O_RDONLY | O_CLOEXEC);
              if (file_fd < 0) {
                perror("Failed to open file");
                close(client_fd);
                continue;
              }

              // Get the file size
              struct stat st;
              if (fstat(file_fd, &st) == -1) {
                perror("Failed to get file stats");
                close(client_fd);
                close(file_fd);
                continue;
              }
              off_t file_size = st.st_size;

              // Send HTTP response header
              char header[256];
              snprintf(header, sizeof(header),
                       "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/octet-stream\r\n"
                       "Content-Length: %ld\r\n"
                       "Connection: close\r\n\r\n",
                       file_size);
              if (send(client_fd, header, strlen(header), 0) == -1) {
                perror("Failed to send HTTP header");
                close(client_fd);
                close(file_fd);
                continue;
              }

              // Initialize connection context
              connection_context *ctx = add_context(client_fd, file_fd);
              if (!ctx) {
                perror("No available context slots");
                close(client_fd);
                close(file_fd);
                continue;
              }

              // Modify epoll events to watch for writable state
              ev.events = EPOLLOUT;
              ev.data.fd = client_fd;
              if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) != 0) {
                perror("epoll_ctl");
                close(client_fd);
                close(file_fd);
                continue;
              }
            } else if (strstr(buf, "GET /") != NULL) {
              const char *response = "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 13\r\n"
                                     "Connection: close\r\n\r\n"
                                     "Hello, World!";
              send(client_fd, response, strlen(response), 0);
            }
          } else if (n == 0) {
            // Client disconnected
            close(client_fd);
            rm_context(client_fd);
          }
        } else if (events[i].events & EPOLLOUT) {
          // Retrieve context
          connection_context *ctx = get_context(client_fd);
          if (!ctx)
            continue; // No context found, skip

          // Use sendfile() to send file content
          ssize_t bytes_sent =
              sendfile(client_fd, ctx->file_fd, &ctx->offset, CHUNK_SIZE);
          if (bytes_sent > 0) {
            // Data sent successfully, continue
            continue;
          } else if (bytes_sent == 0 || errno == EPIPE) {
            // Transfer complete or client disconnected
            close(client_fd);
            rm_context(client_fd); // Free the context
          } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Wait for socket to become writable again, no need to retry
            // immediately
            fprintf(stderr, "Socket buffer full, waiting for EPOLLOUT\n");
            continue;
          } else {
            // Unexpected error, clean up and close the connection
            perror("Error sending file with sendfile");
            close(client_fd);
            rm_context(client_fd);
          }
        }
      }
    }
  }
  close(server_fd);
  close(epoll_fd);
  return 0;
}

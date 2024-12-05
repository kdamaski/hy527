#define _GNU_SOURCE
// #include <errno.h>
#include <fcntl.h>
#include <libaio.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9999
#define MAX_EVENTS 32768
#define BUFFER_SIZE 8192

typedef struct {
  int client_fd;
  int file_fd;
  char *buffer;
  off_t offset;
  size_t file_size;
} client_context_t;

void error_exit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int setup_server_socket() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
    error_exit("socket");

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    error_exit("setsockopt");

  struct sockaddr_in server_addr = {
      .sin_family = AF_INET,
      .sin_addr.s_addr = INADDR_ANY,
      .sin_port = htons(PORT),
  };

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1)
    error_exit("bind");

  if (listen(server_fd, 10) == -1)
    error_exit("listen");

  return server_fd;
}

void handle_large_file(io_context_t ctx, client_context_t *context) {
  struct iocb cb, *cbs[1];
  memset(&cb, 0, sizeof(cb));

  cb.aio_fildes = context->file_fd;
  io_prep_pread(&cb, context->file_fd, context->buffer, BUFFER_SIZE,
                context->offset);

  cbs[0] = &cb;

  int ret = io_submit(ctx, 1, cbs);
  if (ret < 0)
    error_exit("io_submit");

  struct io_event events[1];
  ret = io_getevents(ctx, 1, 1, events, NULL);
  if (ret < 0)
    error_exit("io_getevents");

  ssize_t bytes_read = events[0].res;
  if (bytes_read <= 0) {
    close(context->file_fd);
    close(context->client_fd);
    free(context->buffer);
    free(context);
    return;
  }

  context->offset += bytes_read;
  ssize_t bytes_sent = send(context->client_fd, context->buffer, bytes_read, 0);

  if (bytes_sent <= 0) {
    perror("Error sending data");
    close(context->file_fd);
    close(context->client_fd);
    free(context->buffer);
    free(context);
    return;
  }

  // Update offset only after successful send
  context->offset += bytes_sent;

  if (context->offset >= context->file_size) {
    close(context->file_fd);
    close(context->client_fd);
    free(context->buffer);
    free(context);
  }
}

void event_loop(int server_fd) {
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1)
    error_exit("epoll_create1");

  struct epoll_event event, events[MAX_EVENTS];
  event.events = EPOLLIN;
  event.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    error_exit("epoll_ctl");

  io_context_t ctx;
  memset(&ctx, 0, sizeof(io_context_t));
  if (io_setup(MAX_EVENTS, &ctx) < 0)
    error_exit("io_setup");

  while (1) {
    int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (num_events < 0)
      error_exit("epoll_wait");

    for (int i = 0; i < num_events; ++i) {
      if (events[i].data.fd == server_fd) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd =
            accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1)
          error_exit("accept");

        // printf("Accepted connection: %d\n", client_fd);
        struct epoll_event client_event = {
            .events = EPOLLIN,
            .data.fd = client_fd,
        };
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
          error_exit("epoll_ctl client_fd");
      } else {
        int client_fd = events[i].data.fd;

        char request[1024];
        ssize_t len = recv(client_fd, request, sizeof(request), 0);
        if (len <= 0) {
          close(client_fd);
          continue;
        }

        if (strncmp(request, "GET /large_file", 15) == 0) {
          int file_fd = open("largefile0", O_RDONLY);
          if (file_fd < 0)
            error_exit("open large_file");

          off_t file_size = lseek(file_fd, 0, SEEK_END);
          lseek(file_fd, 0, SEEK_SET);

          client_context_t *context = calloc(1, sizeof(client_context_t));
          context->client_fd = client_fd;
          context->file_fd = file_fd;
          context->buffer = malloc(BUFFER_SIZE);
          context->offset = 0;
          context->file_size = file_size;

          handle_large_file(ctx, context);
        } else {
          const char *response =
              "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
          send(client_fd, response, strlen(response), 0);
          close(client_fd);
        }
      }
    }
  }

  io_destroy(ctx);
  close(epoll_fd);
}

int main() {
  int server_fd = setup_server_socket();
  printf("Server listening on port %d\n", PORT);
  event_loop(server_fd);
  close(server_fd);
  return 0;
}


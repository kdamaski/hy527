#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 9999
#define BACKLOG 128
#define BUFFER_SIZE 8192

void echo_write_cb(struct bufferevent *bev, void *ctx) {
  // Clean up once data is written
  bufferevent_free(bev);
}

void echo_read_cb(struct bufferevent *bev, void *ctx) {
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  // Read the request line
  char *request = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
  if (!request) {
    evbuffer_add_printf(
        output, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
    bufferevent_disable(bev, EV_READ | EV_WRITE);
    bufferevent_free(bev);
    return;
  }

  // Parse the request
  char method[16];
  char uri[256];
  char protocol[16];

  if (sscanf(request, "%15s %255s %15s", method, uri, protocol) != 3) {
    evbuffer_add_printf(
        output, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
    free(request);
    bufferevent_disable(bev, EV_READ | EV_WRITE);
    bufferevent_free(bev);
    return;
  }

  // Only handle GET requests
  if (strcasecmp(method, "GET") != 0) {
    evbuffer_add_printf(
        output, "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n");
    free(request);
    bufferevent_disable(bev, EV_READ | EV_WRITE);
    bufferevent_free(bev);
    return;
  }

  if (strcmp(uri, "/") == 0) {
    // Serve a simple response for the root URI
    evbuffer_add_printf(output, "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 13\r\n"
                                "Connection: close\r\n\r\n"
                                "Hello, World!");
  } else if (strcmp(uri, "/large_file") == 0) {
    // Serve the large file
    const char *file_path = "largefile0";
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
      evbuffer_add_printf(
          output,
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
      free(request);
      return;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
      evbuffer_add_printf(
          output,
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
      close(fd);
      free(request);
      return;
    }
    size_t file_size = st.st_size;

    // Memory-map the file
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
      evbuffer_add_printf(
          output,
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
      close(fd);
      free(request);
      return;
    }

    // Send headers
    evbuffer_add_printf(output,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-Length: %ld\r\n"
                        "Connection: close\r\n\r\n",
                        file_size);

    // Send file content
    evbuffer_add(output, file_data, file_size);

    // Clean up
    munmap(file_data, file_size);
    close(fd);
  } else {
    // Handle 404 for unknown URIs
    evbuffer_add_printf(output,
                        "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
  }

  // Ensure the response is sent before freeing
  bufferevent_setcb(bev, NULL, NULL, NULL, NULL);

  free(request);
  bufferevent_disable(bev, EV_READ | EV_WRITE);
  bufferevent_setcb(bev, NULL, echo_write_cb, NULL,
                    NULL);           // Register write callback
  bufferevent_enable(bev, EV_WRITE); // Ensure write is enabled
}

// Callback for event errors or connection close
void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
  if (events & BEV_EVENT_ERROR) {
    perror("Error on connection");
  }
  bufferevent_free(bev);
}

// Accept new connections and set up callbacks
void accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
               struct sockaddr *address, int socklen, void *ctx) {
  struct event_base *base = (struct event_base *)ctx;
  struct bufferevent *bev =
      bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int main() {
  struct event_base *base = event_base_new();
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sin.sin_port = htons(PORT);

  struct evconnlistener *listener = evconnlistener_new_bind(
      base, accept_cb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, BACKLOG,
      (struct sockaddr *)&sin, sizeof(sin));

  if (!listener) {
    perror("Could not create a listener");
    return 1;
  }

  printf("Starting event loop\n");
  event_base_dispatch(base);
  printf("Event loop exited\n");
  evconnlistener_free(listener);
  event_base_free(base);
  return 0;
}

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
#define BUFFER_SIZE 4096

void echo_read_cb(struct bufferevent *bev, void *ctx) {
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  // Read the request
  char *request = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
  if (!request) {
    return;
  }

  // Check if the request is for /large_file
  if (strstr(request, "GET /large_file")) {
    const char *headers = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/octet-stream\r\n"
                          "Content-Length: %ld\r\n"
                          "Connection: close\r\n\r\n";

    char file_path[] = "largefile0";
    FILE *file = fopen(file_path, "rb");
    if (!file) {
      evbuffer_add_printf(output, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
      free(request);
      return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Write headers to output buffer
    evbuffer_add_printf(output, headers, file_size);

    // Write file content to output buffer
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
      evbuffer_add(output, buffer, bytes_read);
    }

    fclose(file);
  } else {
    evbuffer_add_printf(output, "HTTP/1.1 404 Not Found\r\n\r\n");
  }

  free(request);
  bufferevent_disable(bev, EV_READ | EV_WRITE);
  bufferevent_free(bev);
}

// Callback for event errors or connection close
void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
  // if (events & BEV_EVENT_ERROR) {
  //   perror("Error on connection");
  // }
  // if (events & BEV_EVENT_EOF) {
  //   printf("Client disconnected\n");
  // }
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

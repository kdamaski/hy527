#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9999
#define BACKLOG 128

// Callback for read events
void echo_read_cb(struct bufferevent *bev, void *ctx) {
  char buffer[1024];
  int n = bufferevent_read(bev, buffer, sizeof(buffer) - 1);
  if (n > 0) {
    // buffer[n] = '\0'; // Null-terminate the received data
    // printf("Received request:\n%s\n", buffer); // Log the incoming request

    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Length: 13\r\n"
                           "\r\n"
                           "Hello, World!";
    bufferevent_write(bev, response, strlen(response));
    // printf("Sent response\n");
  }
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

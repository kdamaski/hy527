#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <uv.h>

#define PORT 9999
#define BUFFER_SIZE 8192

typedef struct {
  uv_tcp_t handle;
  uv_write_t write_req;
  uv_buf_t buffer;
  char request[BUFFER_SIZE];
} client_t;

void on_close(uv_handle_t *handle) {
  client_t *client = (client_t *)handle->data;
  if (client) {
    free(client->buffer.base);
    free(client);
  }
}

void send_response(uv_stream_t *client, const char *response, size_t len,
                   uv_write_cb write_cb) {
  client_t *cl = (client_t *)client->data;
  cl->buffer = uv_buf_init(malloc(len), len);
  memcpy(cl->buffer.base, response, len);

  cl->write_req.data = client; // Attach client to write request
  uv_write(&cl->write_req, client, &cl->buffer, 1, write_cb);
}

void after_write(uv_write_t *req, int status) {
  uv_stream_t *client = (uv_stream_t *)req->data;
  uv_close((uv_handle_t *)client, on_close);
}

void on_file_read(uv_fs_t *req) {
  uv_stream_t *client = (uv_stream_t *)req->data;
  uv_fs_req_cleanup(req);

  if (req->result < 0) {
    fprintf(stderr, "File read error: %s\n", uv_strerror(req->result));
    const char *response =
        "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
    send_response(client, response, strlen(response), after_write);
  } else {
    const char *headers =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/octet-stream\r\nConnection: close\r\n\r\n";
    uv_buf_t buffers[2] = {
        uv_buf_init((char *)headers, strlen(headers)),
        uv_buf_init(req->ptr, req->result),
    };

    uv_write_t *write_req = malloc(sizeof(uv_write_t));
    write_req->data = client;

    uv_write(write_req, client, buffers, 2, after_write);
  }

  free(req->ptr);
  free(req);
}

void on_file_stat(uv_fs_t *req) {
  uv_stream_t *client = (uv_stream_t *)req->data;

  if (req->result < 0) {
    fprintf(stderr, "Error stating file: %s\n", uv_strerror(req->result));
    send_response(client, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 35,
                  after_write);
    uv_fs_req_cleanup(req);
    free(req);
    return;
  }

  size_t file_size = req->statbuf.st_size;
  uv_buf_t *buffer = malloc(sizeof(uv_buf_t));
  if (!buffer || file_size == 0) {
    fprintf(stderr, "Failed to allocate buffer or file is empty\n");
    send_response(client, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 35,
                  after_write);
    uv_fs_req_cleanup(req);
    free(req);
    free(buffer);
    return;
  }

  buffer->base = malloc(file_size);
  buffer->len = file_size;

  uv_fs_t *read_req = malloc(sizeof(uv_fs_t));
  if (!read_req) {
    fprintf(stderr, "Failed to allocate read_req\n");
    send_response(client, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 35,
                  after_write);
    free(buffer->base);
    free(buffer);
    uv_fs_req_cleanup(req);
    free(req);
    return;
  }

  read_req->data = client; // Pass client for read callback
  uv_fs_read(uv_default_loop(), read_req, req->result, buffer, file_size, 0,
             on_file_read);

  uv_fs_req_cleanup(req); // Cleanup stat_req
  free(req);
}

void on_file_open(uv_fs_t *req) {
  uv_stream_t *client = (uv_stream_t *)req->data;

  if (req->result < 0) {
    fprintf(stderr, "Error opening file: %s\n", uv_strerror(req->result));
    send_response(client, "HTTP/1.1 404 Not Found\r\n\r\n", 26, after_write);
    uv_fs_req_cleanup(req);
    free(req);
    return;
  }

  int file_fd = req->result; // File descriptor
  uv_fs_t *stat_req = malloc(sizeof(uv_fs_t));
  if (!stat_req) {
    fprintf(stderr, "Failed to allocate stat_req\n");
    send_response(client, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 35,
                  after_write);
    uv_fs_close(uv_default_loop(), req, file_fd, NULL);
    uv_fs_req_cleanup(req);
    free(req);
    return;
  }

  stat_req->data = client; // Pass client for stat callback
  uv_fs_fstat(uv_default_loop(), stat_req, file_fd, on_file_stat);
  uv_fs_req_cleanup(req); // Cleanup open_req
  free(req);
}

void serve_large_file(uv_stream_t *client) {
  uv_fs_t *open_req = malloc(sizeof(uv_fs_t));
  if (!open_req) {
    fprintf(stderr, "Failed to allocate open_req\n");
    send_response(client, "HTTP/1.1 500 Internal Server Error\r\n\r\n", 35,
                  after_write);
    return;
  }

  // Open the file
  uv_fs_open(uv_default_loop(), open_req, "largefile0", O_RDONLY, 0,
             on_file_open);
  open_req->data = client; // Pass client as data for callback
}

void parse_request(uv_stream_t *client, const char *request) {
  char method[16], uri[256], protocol[16];
  if (sscanf(request, "%15s %255s %15s", method, uri, protocol) != 3) {
    const char *response =
        "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
    send_response(client, response, strlen(response), after_write);
    return;
  }

  if (strcasecmp(method, "GET") != 0) {
    const char *response =
        "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
    send_response(client, response, strlen(response), after_write);
    return;
  }

  if (strcmp(uri, "/") == 0) {
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 13\r\n"
                           "Connection: close\r\n\r\n"
                           "Hello, World!";
    send_response(client, response, strlen(response), after_write);

  } else if (strcmp(uri, "/large_file") == 0) {
    serve_large_file(client);

  } else {
    const char *response =
        "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
    send_response(client, response, strlen(response), after_write);
  }
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    uv_close((uv_handle_t *)client, on_close);
    free(buf->base);
    return;
  }

  client_t *cl = (client_t *)client->data;
  memcpy(cl->request, buf->base, nread);
  cl->request[nread] = '\0';

  parse_request(client, cl->request);
  free(buf->base);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  *buf = uv_buf_init(malloc(suggested_size), suggested_size);
}

void on_new_connection(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);

  if (uv_accept(server, (uv_stream_t *)client) == 0) {
    client_t *cl = malloc(sizeof(client_t));
    client->data = cl;

    uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
  } else {
    uv_close((uv_handle_t *)client, NULL);
  }
}

int main() {
  uv_tcp_t server;
  uv_tcp_init(uv_default_loop(), &server);

  struct sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", PORT, &addr);
  uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

  int r = uv_listen((uv_stream_t *)&server, 128, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
    return 1;
  }

  printf("Libuv server listening on port %d...\n", PORT);
  return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

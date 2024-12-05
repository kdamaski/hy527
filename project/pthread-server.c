#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 9999
#define MAX_THREADS 96
#define RCV_SIZE 512
#define SND_SIZE 16384

void handle_client(int client_socket);
void *thread_function(void *arg);

int main() {
  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Create socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  // Bind socket
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("Setsockopt failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_fd, MAX_THREADS) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Thread server listening on port %d...\n", PORT);

  // Accept connections
  while (1) {
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      continue;
    }

    // Create a new thread to handle the client
    pthread_t thread;
    int *client_sock_ptr = malloc(sizeof(int));
    *client_sock_ptr = client_socket;

    if (pthread_create(&thread, NULL, thread_function, client_sock_ptr) != 0) {
      perror("Thread creation failed");
      close(client_socket);
      free(client_sock_ptr);
    }
    // Automatically clean up the thread after it finishes
    pthread_detach(thread);
  }

  close(server_fd);
  return 0;
}

void *thread_function(void *arg) {
  int client_socket = *(int *)arg;
  free(arg);

  handle_client(client_socket);

  close(client_socket);
  return NULL;
}

void handle_client(int client_socket) {
  char buffer[RCV_SIZE];
  ssize_t read_size;

  // Read the client's request
  read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
  if (read_size <= 0) {
    return;
  }
  buffer[read_size] = '\0';

  // Parse the request line
  char method[16], uri[256], protocol[16];
  if (sscanf(buffer, "%15s %255s %15s", method, uri, protocol) != 3) {
    const char *response =
        "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
    return;
  }

  // Only handle GET requests
  if (strcasecmp(method, "GET") != 0) {
    const char *response =
        "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
    return;
  }

  // Handle the request based on URI
  if (strcmp(uri, "/") == 0) {
    // Respond with "Hello, World!"
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 13\r\n"
                           "Connection: close\r\n\r\n"
                           "Hello, World!";
    send(client_socket, response, strlen(response), 0);

  } else if (strcmp(uri, "/large_file") == 0) {
    // Respond with the large file
    const char *file_path = "largefile0";
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
      const char *response =
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
      send(client_socket, response, strlen(response), 0);
      return;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
      close(fd);
      const char *response =
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
      send(client_socket, response, strlen(response), 0);
      return;
    }
    size_t file_size = st.st_size;

    // Send HTTP headers
    char headers[SND_SIZE];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             file_size);
    send(client_socket, headers, strlen(headers), 0);

    // Send the file content
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
      close(fd);
      const char *response =
          "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
      send(client_socket, response, strlen(response), 0);
      return;
    }
    send(client_socket, file_data, file_size, 0);

    munmap(file_data, file_size);
    close(fd);

  } else {
    // Respond with 404 Not Found
    const char *response =
        "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
  }
}

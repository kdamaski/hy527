#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 9999
#define THREAD_POOL_SIZE 4
#define RCV_SIZE 512
#define SND_SIZE 16384
#define REQ_SIZE 65536

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct {
  int client_socket;
  struct sockaddr_in client_addr;
} client_request;

client_request *queue[REQ_SIZE];
int queue_size = 0;
int head = 0, tail = 0;

void enqueue(client_request *req) {
  pthread_mutex_lock(&lock);
  queue[tail] = req;
  tail = (tail + 1) % REQ_SIZE;
  queue_size++;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
}

client_request *dequeue() {
  pthread_mutex_lock(&lock);
  while (queue_size == 0) {
    pthread_cond_wait(&cond, &lock);
  }
  client_request *req = queue[head];
  head = (head + 1) % REQ_SIZE;
  queue_size--;
  pthread_mutex_unlock(&lock);
  return req;
}

void *worker_thread(void *arg) {
  // TODO initialize the local uthread pool

  while (1) {
    client_request *req = dequeue();

    char buffer[RCV_SIZE];
    // memset(buffer, 0, RCV_SIZE);

    // Read HTTP request
    read(req->client_socket, buffer, RCV_SIZE);
    // printf("Received request:\n%s\n", buffer);
    char method[16], uri[256], protocol[16];
    if (sscanf(buffer, "%15s %255s %15s", method, uri, protocol) != 3) {
      const char *response =
          "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
      send(req->client_socket, response, strlen(response), 0);
      return NULL;
    }

    // Only handle GET requests
    if (strcasecmp(method, "GET") != 0) {
      const char *response =
          "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
      send(req->client_socket, response, strlen(response), 0);
      return NULL;
    }

    // Handle the request based on URI
    if (strcmp(uri, "/") == 0) {
      // Respond with "Hello, World!"
      const char *response = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Content-Length: 13\r\n"
                             "Connection: close\r\n\r\n"
                             "Hello, World!";
      send(req->client_socket, response, strlen(response), 0);

    } else if (strcmp(uri, "/large_file") == 0) {
      // Respond with the large file
      const char *file_path = "largefile0";
      int fd = open(file_path, O_RDONLY);
      if (fd < 0) {
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
        send(req->client_socket, response, strlen(response), 0);
        return NULL;
      }

      // Get the file size
      struct stat st;
      if (fstat(fd, &st) < 0) {
        close(fd);
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
        send(req->client_socket, response, strlen(response), 0);
        return NULL;
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
      send(req->client_socket, headers, strlen(headers), 0);

      // Send the file content
      char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
      if (file_data == MAP_FAILED) {
        close(fd);
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
        send(req->client_socket, response, strlen(response), 0);
        return NULL;
      }
      send(req->client_socket, file_data, file_size, 0);

      munmap(file_data, file_size);
      close(fd);

    } else {
      // Respond with 404 Not Found
      const char *response =
          "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
      send(req->client_socket, response, strlen(response), 0);
    }
  }
}

int main() {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, SOMAXCONN) < 0) {
    perror("Listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  pthread_t threads[THREAD_POOL_SIZE];
  for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    pthread_create(&threads[i], NULL, worker_thread, NULL);
  }

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
      perror("Accept failed");
      continue;
    }

    client_request *req = malloc(sizeof(client_request));
    req->client_socket = client_socket;
    req->client_addr = client_addr;

    enqueue(req);
  }

  close(server_socket);
  return 0;
}

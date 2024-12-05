#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9999
#define THREAD_POOL_SIZE 4
#define BUFFER_SIZE 16384

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct {
  int client_socket;
  struct sockaddr_in client_addr;
} client_request;

client_request *queue[BUFFER_SIZE];
int queue_size = 0;
int head = 0, tail = 0;

void enqueue(client_request *req) {
  pthread_mutex_lock(&lock);
  queue[tail] = req;
  tail = (tail + 1) % BUFFER_SIZE;
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
  head = (head + 1) % BUFFER_SIZE;
  queue_size--;
  pthread_mutex_unlock(&lock);
  return req;
}

void *worker_thread(void *arg) {
  while (1) {
    client_request *req = dequeue();

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Read HTTP request
    read(req->client_socket, buffer, BUFFER_SIZE);
    // printf("Received request:\n%s\n", buffer);

    // Send HTTP response
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 13\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "Hello, World!";
    write(req->client_socket, response, strlen(response));

    // Close the connection
    close(req->client_socket);
    free(req);
  }
  return NULL;
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

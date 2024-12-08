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
#include <unistd.h>

#define PORT 9999

connection_context contexts[CONTEXT_SZ] = {0};

int hash_fd(int client_fd) { return (65537 * client_fd) % CONTEXT_SZ; }

connection_context *get_context(int client_fd) {
  // int hash = hash_fd(client_fd);

  // // Search through the linked list at the hashed index
  // connection_context *current = contexts[hash];
  // while (current) {
  //   if (current->client_fd == client_fd) {
  //     return current; // Found the context
  //   }
  //   current = current->next;
  // }
  // return NULL;
  connection_context *ctx = NULL;
  for (int j = 0; j < CONTEXT_SZ; j++) {
    if (contexts[j].client_fd == client_fd) {
      ctx = &contexts[j];
      break;
    }
  }
  return ctx; // Context not found
}

connection_context *add_context(int client_fd, int file_fd) {
  // connection_context *ctx = get_context(client_fd);
  // if (ctx) {
  //   // context exists
  //   // printf("Context with client_fd = %d already exists\n", client_fd);
  //   return ctx;
  // }
  // // If not found, create a new context
  // ctx = malloc(sizeof(connection_context));
  // if (!ctx) {
  //   perror("Failed to allocate memory for connection_context");
  //   return NULL;
  // }
  // int hash = hash_fd(client_fd);
  // ctx->client_fd = client_fd;
  // ctx->file_fd = file_fd;
  // ctx->offset = 0;
  // ctx->next = contexts[hash]; // Add to the head of the linked list
  // contexts[hash] = ctx;
  // return ctx;
  for (int i = 0; i < CONTEXT_SZ; i++) {
    if (contexts[i].client_fd == 0) {
      contexts[i].client_fd = client_fd;
      contexts[i].file_fd = file_fd;
      return &contexts[i];
    }
  }
  return NULL; // No available context
}

void rm_context(int client_fd) {
  // int hash = hash_fd(client_fd);

  // connection_context *current = contexts[hash];
  // connection_context *prev = NULL;
  // while (current) {
  //   if (current->client_fd == client_fd) {
  //     // Found the context to remove
  //     if (prev) {
  //       prev->next = current->next; // Skip over the current node
  //     } else {
  //       contexts[hash] = current->next; // Remove head of list
  //     }
  //     free(current); // Free memory
  //     return;
  //   }
  //   prev = current;
  //   current = current->next;
  // }
  for (int i = 0; i < CONTEXT_SZ; i++) {
    if (contexts[i].client_fd == client_fd) {
      close(contexts[i].file_fd); // Close the file descriptor
      contexts[i].client_fd = 0;
      contexts[i].file_fd = 0;
      contexts[i].offset = 0;
      break;
    }
  }
}

worker_thread workers[NUM_THREADS];

int next_worker = 0;
void distribute_to_worker(int client_fd) {

  // Choose a worker in a round-robin fashion
  int worker_index = next_worker;
  next_worker = (next_worker + 1) % NUM_THREADS;

  // Pass the client_fd to the worker via its event pipe
  write(workers[worker_index].event_pipe[1], &client_fd, sizeof(client_fd));
}

void initialize_workers() {
  for (int i = 0; i < NUM_THREADS; i++) {
    workers[i].epoll_fd = epoll_create1(0);
    if (workers[i].epoll_fd < 0) {
      perror("Failed to create epoll instance");
      exit(EXIT_FAILURE);
    }

    // Create a pipe for passing events to the worker
    if (pipe(workers[i].event_pipe) < 0) {
      perror("Failed to create event pipe");
      exit(EXIT_FAILURE);
    }

    // Add the pipe's read end to the worker's epoll instance
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = workers[i].event_pipe[0];
    epoll_ctl(workers[i].epoll_fd, EPOLL_CTL_ADD, workers[i].event_pipe[0],
              &ev);

    // Create the worker thread
    pthread_create(&workers[i].thread, NULL, work, &workers[i]);
  }
}

void *work(void *arg) {
  worker_thread *worker = (worker_thread *)arg;
  struct epoll_event ev, events[MAX_EVENTS];

  while (1) {
    int nfds = epoll_wait(worker->epoll_fd, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == worker->event_pipe[0]) {
        // New client_fd from the main thread
        int client_fd;
        if (read(worker->event_pipe[0], &client_fd, sizeof(client_fd)) !=
            sizeof(client_fd)) {
          fprintf(stderr, "Failed to read client_fd from pipe\n");
          continue;
        }

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_fd;
        if (epoll_ctl(worker->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
          perror("epoll_ctl: add client_fd");
          close(client_fd);
          continue;
        }

        // fprintf(stderr, "Worker added client_fd %d\n", client_fd);

      } else {
        int client_fd = events[i].data.fd;

        if (events[i].events & EPOLLIN) {
          char buf[1024];
          int n = recv(client_fd, buf, sizeof(buf) - 1, 0);

          if (n > 0) {
            buf[n] = '\0'; // Null-terminate the received data
            // printf("Worker received: %s\n", buf);

            if (strstr(buf, "GET /large_file") != NULL) {
              // Open the requested file
              int file_fd =
                  open("./largefile0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
              if (file_fd < 0) {
                perror("Failed to open largefile0");
                close(client_fd);
                continue;
              }

              struct stat st;
              if (fstat(file_fd, &st) == -1) {
                perror("Failed to get file stats");
                close(client_fd);
                close(file_fd);
                continue;
              }

              off_t file_size = st.st_size;

              // Send HTTP header
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

              // Add connection context for non-blocking sendfile
              connection_context *ctx = add_context(client_fd, file_fd);
              if (!ctx) {
                perror("No available context slots");
                close(client_fd);
                close(file_fd);
                continue;
              }
              ctx->client_fd = client_fd;
              ctx->file_fd = file_fd;
              ctx->offset = 0;

              // Modify epoll events to watch for writable state
              ev.events = EPOLLOUT;
              ev.data.fd = client_fd;
              epoll_ctl(worker->epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);

            } else if (strstr(buf, "GET /") != NULL) {
              // Handle "Hello, World!" request
              const char *response = "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 13\r\n"
                                     "Connection: close\r\n\r\n"
                                     "Hello, World!";
              send(client_fd, response, strlen(response), 0);
              close(client_fd);
            }
          } else if (n == 0) {
            // Client disconnected
            // fprintf(stderr, "Client disconnected, closing fd %d\n",
            // client_fd);
            close(client_fd);
            rm_context(client_fd);
          } else {
            perror("recv failed");
            close(client_fd);
            rm_context(client_fd);
          }
        }

        if (events[i].events & EPOLLOUT) {
          connection_context *ctx = get_context(client_fd);
          if (!ctx) {
            fprintf(stderr, "No context for client_fd %d\n", client_fd);
            close(client_fd);
            continue;
          }

          // Use sendfile() for non-blocking file transfer
          ssize_t bytes_sent =
              sendfile(client_fd, ctx->file_fd, &ctx->offset, CHUNK_SIZE);
          if (bytes_sent == 0 || errno == EPIPE) {
            // Transfer complete or client disconnected
            fprintf(stderr, "File transfer complete for fd %d\n", client_fd);
            close(client_fd);
            rm_context(client_fd);
          } else if (bytes_sent < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
              perror("sendfile failed");
              close(client_fd);
              rm_context(client_fd);
            }
          }
        }
      }
    }
  }
  return NULL;
}

int main() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(PORT),
                                    .sin_addr.s_addr = INADDR_ANY};

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  // Initialize worker threads
  initialize_workers();

  // Main event loop
  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
      perror("Accept failed");
      continue;
    }

    // Set the new socket to non-blocking mode
    fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

    // Distribute the connection to a worker
    distribute_to_worker(client_fd);
  }

  close(server_fd);
  return 0;
}

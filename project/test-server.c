#include "thread-server.h"
#include "uthread.h"
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 9999
#define MAX_EVENTS 101

connection_context *contexts[CONTEXT_SZ] = {NULL};

// connection_context *get_context(int client_fd) {
//   connection_context *ctx = NULL;
//   for (int i = 0; i < CONTEXT_SZ; i++) {
//     if (contexts[i].client_fd == client_fd) {
//       ctx = &contexts[i];
//       break;
//     }
//   }
//   return ctx; // Context not found
// }

// connection_context *add_context(int client_fd, int file_fd, long file_sz) {
//   for (int i = 0; i < CONTEXT_SZ; i++) {
//     if (contexts[i].client_fd == 0) {
//       contexts[i].client_fd = client_fd;
//       contexts[i].file_fd = file_fd;
//       contexts[i].file_sz = file_sz;
//       return &contexts[i];
//     }
//   }
//   return NULL; // No available context
// }

// void rm_context(int client_fd) {
//   for (int i = 0; i < CONTEXT_SZ; i++) {
//     if (contexts[i].client_fd == client_fd) {
//       close(contexts[i].file_fd); // Close the file descriptor
//       contexts[i].client_fd = 0;
//       contexts[i].file_fd = 0;
//       contexts[i].offset = 0;
//       contexts[i].file_sz = 0;
//       break;
//     }
//   }
// }

int hash_fd(int client_fd) { return (65537 * client_fd) % CONTEXT_SZ; }

connection_context *get_context(int client_fd) {
  int hash = hash_fd(client_fd);

  // Search through the linked list at the hashed index
  connection_context *current = contexts[hash];
  while (current) {
    if (current->client_fd == client_fd) {
      return current; // Found the context
    }
    current = current->next;
  }
  return NULL;
}

connection_context *add_context(int client_fd, int file_fd, long file_size) {
  connection_context *ctx = get_context(client_fd);
  if (ctx) {
    // context exists
    // printf("Context with client_fd = %d already exists\n", client_fd);
    return ctx;
  }
  // If not found, create a new context
  ctx = malloc(sizeof(connection_context));
  if (!ctx) {
    perror("Failed to allocate memory for connection_context");
    return NULL;
  }
  int hash = hash_fd(client_fd);
  ctx->client_fd = client_fd;
  ctx->file_fd = file_fd;
  ctx->offset = 0;
  ctx->file_sz = file_size;
  ctx->next = contexts[hash]; // Add to the head of the linked list
  contexts[hash] = ctx;
  return ctx;
}

void rm_context(int client_fd) {
  int hash = hash_fd(client_fd);

  connection_context *current = contexts[hash];
  connection_context *prev = NULL;
  while (current) {
    if (current->client_fd == client_fd) {
      // Found the context to remove
      if (prev) {
        prev->next = current->next; // Skip over the current node
      } else {
        contexts[hash] = current->next; // Remove head of list
      }
      free(current); // Free memory
      return;
    }
    prev = current;
    current = current->next;
  }
}

void *kthread_work(void *arg);
worker_thread workers[NUM_THREADS];

void start_workers() {
  for (int i = 0; i < NUM_THREADS; i++) {
    // init per kernel thread local queue
    workers[i].epoll_fd = epoll_create1(0);
    if (workers[i].epoll_fd < 0) {
      perror("Failed to create epoll instance");
      exit(EXIT_FAILURE);
    }

    // make the fd non-blocking
    fcntl(workers[i].epoll_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);
    if (pipe(workers[i].event_pipe) < 0) {
      perror("Failed to create event pipe");
      exit(EXIT_FAILURE);
    }

    // Add the pipe's read end to the worker's epoll instance
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = workers[i].event_pipe[0];
    // fcntl(workers[i].event_pipe[0], F_SETFL, O_NONBLOCK);
    epoll_ctl(workers[i].epoll_fd, EPOLL_CTL_ADD, workers[i].event_pipe[0],
              &ev);

    pthread_create(&workers[i].thread, NULL, kthread_work, &workers[i]);
  }
}

void *uthread_work(void *arg) {
  worker_thread *worker = (worker_thread *)arg;
  int client_fd = ((struct u_thread *)(worker->uq->curr_thr->id))->client_fd;

  connection_context *ctx = NULL;
  char buf[256];

  while (1) {
    // Check if we're sending a file
    if (ctx && ctx->file_fd > 0) {
      // Use sendfile() to send the file in chunks
      ssize_t bytes_sent =
          sendfile(client_fd, ctx->file_fd, &ctx->offset, ctx->file_sz);
      if (bytes_sent == 0 || errno == EPIPE) {
        // File transfer complete or client disconnected
        // printf("File transfer complete for fd %d\n", client_fd);
        close(client_fd);
        rm_context(client_fd); // Cleanup context
        break;
      } else if (bytes_sent < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          // perror("sendfile failed");
          close(client_fd);
          rm_context(client_fd);
          break;
        }
      }

      // Pause the user thread; wait for EPOLLOUT if needed
      Thread_pause(worker->uq);

      continue; // Retry sending when unpaused
    }

    // Handle incoming data
    int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
      buf[n] = '\0'; // Null-terminate the string
      // printf("User thread received: %s\n", buf);

      if (strstr(buf, "GET /large_file") != NULL) {
        // Open the requested file
        int file_fd;
        if (buf[15] == '2') {
          file_fd = open("./largefile6", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        } else {
          file_fd = open("./largefile0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        }
        if (file_fd < 0) {
          perror("Failed to open largefile0");
          close(client_fd);
          break;
        }

        struct stat st;
        if (fstat(file_fd, &st) == -1) {
          perror("Failed to get file stats");
          close(client_fd);
          close(file_fd);
          break;
        }

        off_t file_size = st.st_size;

        // Send HTTP header
        char header[128];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n\r\n",
                 file_size);
        if (send(client_fd, header, strlen(header), 0) <= 0) {
          perror("Failed to send HTTP header");
          close(client_fd);
          close(file_fd);
          break;
        }

        // Create context for non-blocking file transfer
        ctx = add_context(client_fd, file_fd, file_size);
        if (!ctx) {
          perror("No available context slots");
          close(client_fd);
          close(file_fd);
          break;
        }

        // Modify epoll events to watch for writable state
        struct epoll_event ev;
        ev.events = EPOLLOUT; // Switch to write mode
        ev.data.fd = client_fd;

        if (epoll_ctl(worker->epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
          perror("epoll_ctl: switch to EPOLLOUT");
          close(client_fd);
          rm_context(client_fd);
          break;
        }

        // Pause the user thread until the connection is writable
        Thread_pause(worker->uq);
      } else if (strstr(buf, "GET /") != NULL) {
        // Handle "Hello, World!" request
        const char *response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 13\r\n"
                               "Connection: close\r\n\r\n"
                               "Hello, World!";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
        break;
      }
    } else if (n == 0) {
      // Client disconnected
      // printf("Client disconnected: fd %d\n", client_fd);
      close(client_fd);
      break;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available; user thread can pause
      Thread_pause(worker->uq);
    } else {
      perror("recv failed");
      close(client_fd);
      break;
    }
  }

  return worker->uq; // Needed in thread_exit
}

void *kthread_work(void *arg) {
  worker_thread *worker = (worker_thread *)arg;
  worker->uq = Thread_init();

  struct epoll_event events[MAX_EVENTS], ev;

  while (1) {
    int nfds = epoll_wait(worker->epoll_fd, events, MAX_EVENTS, -1);

    for (int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;

      // Check if the event came from the event pipe (new connections)
      if (fd == worker->event_pipe[0]) {
        int client_fd;
        if (read(worker->event_pipe[0], &client_fd, sizeof(client_fd)) ==
            sizeof(client_fd)) {
          // Add the new connection to epoll
          struct epoll_event ev;
          ev.events = EPOLLIN; // Initially wait for input
          ev.data.fd = client_fd;

          if (epoll_ctl(worker->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
            perror("epoll_ctl: add client_fd");
            close(client_fd);
            continue;
          }

          // Create a user thread for this connection
          u_thread *uthread = (u_thread *)Thread_new(
              uthread_work, worker, sizeof(worker_thread), worker->uq);
          uthread->client_fd = client_fd;
        } else {
          fprintf(stderr, "Failed to read client_fd from pipe\n");
          continue;
        }

      } else {
        // The event is from an existing connection
        if (events[i].events & (EPOLLIN | EPOLLOUT)) {
          Thread_pause(worker->uq);
        }
        if (events[i].events & (EPOLLERR | EPOLLHUP)) {
          // Cleanup the connection
          // fprintf(stderr, "Entered EPOLLER or EPOLLHUP\n");
          close(fd);
          epoll_ctl(worker->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
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
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
             sizeof(opt));

  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(PORT),
                                    .sin_addr.s_addr = INADDR_ANY};

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 32) < 0) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  start_workers();

  int next_worker = 0;
  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
      perror("Accept failed");
      continue;
    }

    // Set the new socket to non-blocking mode
    fcntl(client_fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);

    // Distribute the connection to a worker

    // Choose a worker in a round-robin fashion
    int worker_index = next_worker;
    next_worker = (next_worker + 1) % NUM_THREADS;

    // Pass the client_fd to the worker via its event pipe
    if (write(workers[worker_index].event_pipe[1], &client_fd,
              sizeof(client_fd)) == -1) {
      perror("Failed to write fd into workers' event pipe");
      exit(EXIT_FAILURE);
    }
  }

  close(server_fd);
  return 0;
}

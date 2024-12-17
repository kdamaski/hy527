#include "uthread.h"
#include <fcntl.h>
#include <pthread.h>

#define NUM_THREADS 4
#define CONTEXT_SZ 503
// 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
// 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373,
// 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457,
// 461, 463, 467, 479, 487, 491, 499, 503

// #define CHUNK_SIZE 65536

typedef struct connection_context {
  int client_fd;
  int file_fd;
  off_t offset;
  long file_sz;
  struct connection_context *next; // Linked list for collision resolution
} connection_context;

connection_context *add_context(int client_fd, int file_fd, long file_sz);

connection_context *get_context(int client_fd);

void rm_context(int client_fd);

// Worker thread structure
typedef struct worker_thread {
  int epoll_fd;      // epoll instance for this thread
  pthread_t thread;  // Thread ID
  int event_pipe[2]; // Pipe for passing new events to this worker
  uthread_queue *uq;
} worker_thread;

void initialize_workers();

void distribute_to_workers(int client_fd);

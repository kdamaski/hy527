#include <fcntl.h>
#include <pthread.h>

#define CONTEXT_SZ 179
#define NUM_THREADS 4

typedef struct connection_context {
  int client_fd;
  int file_fd;
  off_t offset;
  long file_size;
} connection_context;

connection_context *add_context(int client_fd, int file_fd, long file_size);

connection_context *get_context(int client_fd);

void rm_context(int client_fd);
//
// Worker thread structure
typedef struct worker_thread {
  int epoll_fd;      // epoll instance for this thread
  pthread_t thread;  // Thread ID
  int event_pipe[2]; // Pipe for passing new events to this worker
} worker_thread;

void initialize_workers();

void distribute_to_workers(int client_fd);

// the worker function
void *work(void *arg);

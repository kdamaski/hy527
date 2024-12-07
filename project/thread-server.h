#include <fcntl.h>

#define MAX_EVENTS 2048
#define CONTEXT_SZ 307
#define CHUNK_SIZE 131072

typedef struct connection_context {
  int client_fd;
  int file_fd;
  off_t offset;
  // struct connection_context *next; // Linked list for collision resolution
} connection_context;

int hash_fd(int client_fd);

connection_context *add_context(int client_fd, int file_fd);

connection_context *get_context(int client_fd);

void rm_context(int client_fd);

// the worker function
void *worker_thread(void *arg);

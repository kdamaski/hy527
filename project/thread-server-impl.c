#include "thread-server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void *worker_thread(void *arg) {}

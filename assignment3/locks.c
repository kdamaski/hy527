#include "atomic.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// #define MAX_SPIN 12332
// #define MIN_SPIN 932

/*     spinlock start        */
// void backoff() {
//   unsigned spins = rand() % (MAX_SPIN - MIN_SPIN) + MIN_SPIN;
//   for (int i = 0; i < spins; i++) {
//     ;
//   }
// }

spinlock_t s_lock;

void spinlock_init() { s_lock.lock = 0; }

void spinlock_lock(spinlock_t *lock) {
  while (1) {
    // if lock found 0 (unlocked) then set it to 1 (locked) and return
    if (CMPXCHG(&lock->lock, 0, 1) == 0) { // compared with initial lock val
      return;
    }
    // Reduce CPU contention using a 'pause' instruction
    __asm__ volatile("pause");
  }
}
// since lock sets the value 1. Decrementing is enough to unlock
void spinlock_unlock(spinlock_t *lock) { DECREMENT(lock->lock); }
/*     spinlock end        */

/*     ticketlock start        */
ticket_lock_t t_lock;

void ticket_lock_init(ticket_lock_t *lock) {
  lock->next_ticket = 0;
  lock->curr_ticket = 0;
}

void ticket_lock(ticket_lock_t *lock) {
  unsigned my_ticket;
  FETCH_AND_INCREMENT(lock->next_ticket,
                      my_ticket); // Get the next ticket and increment

  // Wait until the serving ticket matches our ticket
  while (lock->curr_ticket != my_ticket) {
    // Reduce CPU contention using a 'pause' instruction
    __asm__ volatile("pause");
  }
}

void ticket_unlock(ticket_lock_t *lock) {
  INCREMENT(lock->curr_ticket); // Increment ticket allowing lock of the next
}
/*     ticketlock end        */

/* mcs_lock  start */
mcs_lock_t mcslock;
void mcs_lock_init(mcs_lock_t *lock) {
  lock->tail = NULL; // Initialize the tail to null (no threads in the queue)
}

void mcs_lock(mcs_lock_t *lock, mcs_node_t *my_node) {
  my_node->next = NULL; // Initialize next pointer
  my_node->locked = 1;  // Mark the node as locked

  // Atomically swap the tail with the current node (enqueue this node)
  mcs_node_t *prev = (mcs_node_t *)CMPXCHG(&lock->tail, (unsigned long)NULL,
                                           (unsigned long)my_node);

  printf(
      "Managed to survive CMPXCHG prev was %p tail is %p and has locked = %u\n",
      prev, mcslock.tail, mcslock.tail->locked);
  if (prev != NULL) {
    // Queue was not empty, link this node to the previous one
    prev->next = my_node;

    // Spin until this thread is granted the lock (locked becomes 0)
    while (my_node->locked == 1) {
      __asm__ volatile("pause");
    }
  }
}

void mcs_unlock(mcs_lock_t *lock, mcs_node_t *my_node) {
  // Check if there is a successor node
  if (my_node->next == NULL) {
    // Try to set the tail to NULL if this node is the last one
    if (CMPXCHG(&lock->tail, NULL, NULL) == (unsigned long)my_node) {
      return; // Lock released with no successor
    }

    // Wait until the next pointer is updated by a new enqueued node
    while (my_node->next == NULL) {
      __asm__ volatile("pause");
    }
  }

  // Notify the next node in the queue by setting its locked flag to 0
  my_node->next->locked = 0;
}

int shared_counter = 0;
void *thread_func(void *arg) {
  mcs_node_t my_node; // Each thread has its own node

  for (int i = 0; i < 10000; i++) {
    // ticket_lock(&t_lock);
    // spinlock_lock(&s_lock);
    mcs_lock(&mcslock, &my_node);

    shared_counter++;
    // spinlock_unlock(&s_lock);
    // ticket_unlock(&t_lock);
    mcs_unlock(&mcslock, &my_node);
  }
  printf("counter out %d\n", shared_counter);

  return NULL;
}

/* mcs_lock  end */

int main(int argc, char *argv[]) {
  mcs_lock_init(&mcslock);
  ticket_lock_init(&t_lock);
  spinlock_init();

  // Create threads
  const int num_threads = 4;
  pthread_t threads[num_threads];

  for (int i = 0; i < num_threads; i++) {
    if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
      perror("Failed to create thread");
      return 1;
    }
  }

  // Join threads
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  // Print the result
  printf("Final counter value: %d\n", shared_counter);
  return EXIT_SUCCESS;
}

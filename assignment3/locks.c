#include "atomic.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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

void mcs_lock_init() { mcslock.tail = NULL; }

void mcs_lock(mcs_node_t *my_node) {

  my_node->next = NULL;
  my_node->locked = 1;

  mcs_node_t *prev =
      (mcs_node_t *)EXCHANGE(&mcslock.tail, (unsigned long)my_node);

  if (prev != NULL) {
    // Queue was not empty, link this node to the previous one
    STORE_EXPLICIT(&prev->next, (unsigned long)my_node);

    // Spin until this thread is granted the lock (locked becomes 0)
    while (my_node->locked) {
      // __asm__ volatile("pause"); // good for more cpu bound ops
      ;
    }
  }
}

void mcs_unlock(mcs_node_t *my_node) {
  // Check if there is a successor node
  mcs_node_t *successor = (mcs_node_t *)LOAD_EXPLICIT(&my_node->next);
  if (successor == NULL) {
    if (CMPXCHG(&mcslock.tail, (unsigned long)my_node, (unsigned long)NULL) ==
        (unsigned long)my_node) {
      return; // Lock released successfully
    }
    // Wait until the next pointer is updated by a new enqueued node
    while ((successor = (mcs_node_t *)LOAD_EXPLICIT(&my_node->next)) == NULL)
      ;
  }

  // Notify the next node in the queue by setting its locked flag to 0
  STORE_EXPLICIT(&successor->locked, 0);
}

int shared_counter = 0;
void *thread_func(void *arg) {
  mcs_node_t my_node; // Each thread has its own node

  for (int i = 0; i < 10000; i++) {
    mcs_lock(&my_node);
    // ticket_lock(&t_lock);
    // spinlock_lock(&s_lock);

    shared_counter++;
    // spinlock_unlock(&s_lock);
    // ticket_unlock(&t_lock);
    mcs_unlock(&my_node);
  }
  printf("counter out %d\n", shared_counter);

  return NULL;
}

/* mcs_lock  end */

int main(int argc, char *argv[]) {
  mcs_lock_init();
  // ticket_lock_init(&t_lock);
  // spinlock_init();

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

#include "atomic.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>

// spinlock_t s_lock;

// void spinlock_init() { s_lock.lock = 0; }

// void spinlock_lock(spinlock_t *lock) {
//   while (1) {
//     // if lock found 0 (unlocked) then set it to 1 (locked) and return
//     if (CMPXCHG(&lock->lock, 0, 1) == 0) { // compared with initial lock val
//       return;
//     }
//     // Reduce CPU contention using a 'pause' instruction
//     __asm__ volatile("pause");
//   }
// }
// // since lock sets the value 1. Decrementing is enough to unlock
// void spinlock_unlock(spinlock_t *lock) { DECREMENT(lock->lock); }
// /*     spinlock end        */

// /*     ticketlock start        */
// ticket_lock_t t_lock;

// void ticket_lock_init(ticket_lock_t *lock) {
//   lock->next_ticket = 0;
//   lock->curr_ticket = 0;
// }

// void ticket_lock_lock(ticket_lock_t *lock) {
//   unsigned my_ticket;
//   FETCH_AND_INCREMENT(lock->next_ticket,
//                       my_ticket); // Get the next ticket and increment

//   // Wait until the serving ticket matches our ticket
//   while (lock->curr_ticket != my_ticket) {
//     // Reduce CPU contention using a 'pause' instruction
//     __asm__ volatile("pause");
//   }
// }

// void ticket_lock_unlock(ticket_lock_t *lock) {
//   INCREMENT(lock->curr_ticket); // Increment ticket allowing lock of the next
// }
// /*     ticketlock end        */

// /* mcs_lock  start */
typedef struct mcs_node {
  volatile struct mcs_node *next;
  unsigned locked; // Is lock occupied? (1 locked, 0 unlocked)
} mcs_node_t;

typedef struct lock_t {
  volatile mcs_node_t *tail;
} mcs_lock_t;
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
/* mcs lock end */

// barrier start
typedef struct {
  volatile unsigned thr_count;
  volatile unsigned bar_phase;
} barrier_t;

// Function to initialize the barrier
void barrier_init(barrier_t *barrier) {
  barrier->thr_count = 0;
  barrier->bar_phase = 0;
}

// Function for threads to wait at the barrier
void barrier_wait(barrier_t *barrier, int num_threads) {
  int old_value;
  int local_phase = barrier->bar_phase;

  // Atomically increment the thr_count of threads at the barrier
  FETCH_AND_INCREMENT(barrier->thr_count, old_value);

  if (barrier->thr_count == num_threads) {
    // Last thread to arrive resets the thr_count and advances the bar_phase
    barrier->thr_count = 0;
    MEMORY_BARRIER();     // Ensure memory operations are completed
    barrier->bar_phase++; // Increment the bar_phase to release waiting threads
  } else {
    // Busy-wait until the bar_phase changes, using 'pause' for efficiency
    while (barrier->bar_phase == local_phase) {
      __asm__ volatile("pause");
    }
  }
}

// barrier end

// int shared_counter = 0;

// lock_t id_lock;

// void *thread_func(void *arg) {

//   for (int i = 0; i < 50000; i++) {
//     lock(&id_lock);
//     // Critical section
//     shared_counter++;
//     unlock(&id_lock);
//   }
//   printf("Thread %d exiting with counter value: %d\n", *(int *)arg,
//          shared_counter);

//   return NULL;
// }

// int main() {
//   pthread_t threads[4];

//   // Initialize the lock
//   lock_init(&id_lock);

//   // Create threads
//   int arg[4] = {0, 1, 2, 3};
//   for (int i = 0; i < 4; i++) {
//     if (pthread_create(&threads[i], NULL, thread_func, (void *)&arg[i]) != 0)
//     {
//       perror("Failed to create thread");
//       return 1;
//     }
//   }

//   // Join threads
//   for (int i = 0; i < 4; i++) {
//     pthread_join(threads[i], NULL);
//   }

//   // Print the result
//   printf("Final counter value: %d\n", shared_counter);

//   return 0;
// }

// int main(int argc, char *argv[]) {
//   mcs_node_t *src, *newval, *output;
//   src = malloc(sizeof(mcs_node_t));
//   printf("src ptr has address : %p\n", src);
//   newval = malloc(sizeof(mcs_node_t));
//   printf("newval ptr has address : %p\n", newval);

//   output = (mcs_node_t *)EXCHANGE(&src, (unsigned long)newval);

//   printf("src ptr has address : %p\n", src);
//   printf("output ptr has address : %p\n", output);
//   return EXIT_SUCCESS;
// }

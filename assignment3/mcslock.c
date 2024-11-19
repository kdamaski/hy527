#include "atomic.h"
#define NULL 0

typedef struct mcs_node {
  volatile struct mcs_node *next;
  unsigned locked; // Is lock occupied? (1 locked, 0 unlocked)
} mcs_node_t;

typedef struct lock_t {
  volatile mcs_node_t *tail;
} mcs_lock_t;

mcs_lock_t mcslock;

void lock_init(void *lock) { ((mcs_lock_t *)lock)->tail = NULL; }

void lock(void *me) {

  mcs_node_t *my_node = (mcs_node_t *)me;
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

void unlock(void *me) {
  mcs_node_t *my_node = (mcs_node_t *)me;
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

__thread mcs_node_t node; // per thread local variable

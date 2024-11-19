#include "atomic.h"

typedef struct lock_t {
  volatile unsigned lock; // Lock variable (0 for unlocked, 1 for locked)
} spinlock_t;

spinlock_t s_lock;

void lock_init(void *lock) { ((spinlock_t *)lock)->lock = 0; }

void lock(void *lck) {
  spinlock_t *lock = (spinlock_t *)lck;
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
void unlock(void *lck) {
  spinlock_t *lock = (spinlock_t *)lck;
  DECREMENT(lock->lock);
}

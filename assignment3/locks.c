#include "atomic.h"
#include <stdlib.h>

#define MAX_SPIN 34332
#define MIN_SPIN 17632

/*     spinlock start        */
// NOTE rand must be initialized()
void backoff() {
  unsigned spins = rand() % (MAX_SPIN - MIN_SPIN) + MIN_SPIN;
  for (int i = 0; i < spins; i++) {
    ;
  }
}
void spinlock_lock(spinlock_t *lock) {
  while (1) {
    // if lock found 0 -> unlocked then set it to 1 -> locked
    // and return!
    if (CMPXCHG(&lock->lock, 0, 1) == 0) {
      return;
    }
    backoff();
  }
}
// since lock sets the value 1. Decrementing is enough to unlock
void spinlock_unlock(spinlock_t *lock) { DECREMENT(lock->lock); }
/*     spinlock end        */

/*     ticketlock start        */

/*     ticketlock end        */

int main(int argc, char *argv[]) { return EXIT_SUCCESS; }

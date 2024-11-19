#include "atomic.h"

typedef struct lock_t {
  volatile unsigned next_ticket; // The next available ticket number
  volatile unsigned curr_ticket; // The current ticket being served
} ticket_lock_t;

ticket_lock_t t_lock;

void lock_init(void *lock) {
  ((ticket_lock_t *)lock)->next_ticket = 0;
  ((ticket_lock_t *)lock)->curr_ticket = 0;
}

void lock(void *lck) {
  ticket_lock_t *lock = (ticket_lock_t *)lck;
  unsigned my_ticket;
  FETCH_AND_INCREMENT(lock->next_ticket,
                      my_ticket); // Get the next ticket and increment

  // Wait until the serving ticket matches our ticket
  while (lock->curr_ticket != my_ticket) {
    // Reduce CPU contention using a 'pause' instruction
    __asm__ volatile("pause");
  }
}

void unlock(void *lck) {
  ticket_lock_t *lock = (ticket_lock_t *)lck;
  INCREMENT(lock->curr_ticket); // Increment ticket allowing lock of the next
}

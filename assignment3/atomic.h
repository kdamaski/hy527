/*
 * Atomic operations for 4-byte values for the cluster nodes
 * and in gcc inline assembly.
 */

typedef struct {
  volatile unsigned long lock; // Lock variable (0 for unlocked, 1 for locked)
} spinlock_t;

#define FETCH_AND_ADD(address, increment)                                      \
  asm volatile("lock xaddl %0,%1"                                              \
               : "=r"(increment), "=m"(address)                                \
               : "0"(increment)                                                \
               : "memory");

#define FETCH_AND_INCREMENT(value, oldvalue)                                   \
  do {                                                                         \
    oldvalue = 1;                                                              \
    FETCH_AND_ADD(value, oldvalue);                                            \
  } while (0);

#define FETCH_AND_DECREMENT(value, oldvalue)                                   \
  do {                                                                         \
    oldvalue = -1;                                                             \
    FETCH_AND_ADD(value, oldvalue);                                            \
  } while (0);

#define INCREMENT(value)                                                       \
  asm volatile("lock incl %0" : "=m"(value) : "m"(value));

#define DECREMENT(value)                                                       \
  asm volatile("lock decl %0" : "=m"(value) : "m"(value));

// returns the old value.
static inline unsigned long CMPXCHG(volatile void *ptr, unsigned long ovalue,
                                    unsigned long nvalue) {
  unsigned long prev;
  asm volatile("lock cmpxchgl %k1,%2"
               : "=a"(prev)
               : "r"(nvalue), "m"(*(volatile long *)(ptr)), "0"(ovalue)
               : "memory");
  return prev;
}

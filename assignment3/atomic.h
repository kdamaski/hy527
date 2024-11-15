/*
 * Atomic operations for 4-byte values for the cluster nodes
 * and in gcc inline assembly.
 */

// spin lock start
typedef struct {
  volatile unsigned lock; // Lock variable (0 for unlocked, 1 for locked)
} spinlock_t;
// spin lock end

// ticket lock start
typedef struct {
  volatile unsigned next_ticket; // The next available ticket number
  volatile unsigned curr_ticket; // The current ticket being served
} ticket_lock_t;
// ticket lock end

// mcs lock start
typedef struct mcs_node {
  struct mcs_node *next;
  volatile unsigned locked; // Is lock occupied? (1 locked, 0 unlocked)
} mcs_node_t;

typedef struct {
  struct mcs_node *tail;
} mcs_lock_t;
// mcs lock end

#define FETCH_AND_ADD(address, increment)                                      \
  asm volatile("lock xadd %0,%1"                                               \
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

// returns the old value of ptr (lock).
static inline unsigned long CMPXCHG(volatile void *ptr, unsigned long ovalue,
                                    unsigned long nvalue) {
  unsigned long prev;
  asm volatile("lock cmpxchgl %k1,%2"
               : "=a"(prev)
               : "r"(nvalue), "m"(*(volatile long *)(ptr)), "0"(ovalue)
               : "memory");
  return prev;
}

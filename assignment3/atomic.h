/*
 * Atomic operations for 4-byte values for the cluster nodes
 * and in gcc inline assembly.
 */

// barrier start
typedef struct {
  int num_threads;
  volatile unsigned thr_count;
  volatile unsigned bar_phase;
} barrier_t;
// barrier end

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
  volatile struct mcs_node *next;
  unsigned locked; // Is lock occupied? (1 locked, 0 unlocked)
} mcs_node_t;

typedef struct {
  volatile mcs_node_t *tail;
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
  asm volatile("lock cmpxchgq %1,%2"
               : "=a"(prev)
               : "r"(nvalue), "m"(*(volatile unsigned long *)(ptr)), "0"(ovalue)
               : "memory");
  return prev;
}
static inline unsigned long EXCHANGE(volatile void *ptr,
                                     unsigned long new_value) {
  unsigned long old_value;
  asm volatile(
      "xchgq %0, %1" // Atomically exchange `new_value` with the value at `*ptr`
      : "=r"(old_value), // Output: `old_value` will hold the original value at
                         // `*ptr`
        "+m"(*(unsigned long *)
                 ptr)  // Output/Input: `*ptr` is updated with `new_value`
      : "0"(new_value) // Input: `new_value` is placed in the same register as
                       // `old_value`
      : "memory"       // Clobber: informs the compiler that memory is modified
  );
  return old_value; // Return the old value stored at `*ptr`
}

static inline void STORE_EXPLICIT(volatile void *ptr, unsigned long value) {
  asm volatile(
      "movq %1, %0" // Atomically move `value` into `*ptr`
      : "=m"(*(unsigned long *)
                 ptr) // Output: memory location pointed by `ptr` is updated
      : "r"(value)    // Input: value to store is provided in a register
      : "memory"      // Clobber: informs the compiler that memory is modified
  );
}

static inline unsigned long LOAD_EXPLICIT(volatile void *ptr) {
  unsigned long value;
  asm volatile(
      "movq %1, %0" // Atomically move the value at `*ptr` into `value`
      : "=r"(value) // Output: `value` will hold the data from `*ptr`
      : "m"(*(unsigned long *)ptr) // Input: memory location to read from
      : "memory" // Clobber: informs the compiler that memory is read
  );
  return value;
}
static inline void MEMORY_BARRIER() { asm volatile("mfence" ::: "memory"); }

// all the following locks assume that a global lock of each kind will be
// instantiated
void spinlock_init();
void spinlock_lock(spinlock_t *lock);
void spinlock_unlock(spinlock_t *lock);

void ticket_lock_init(ticket_lock_t *lock);
void ticket_lock(ticket_lock_t *lock);
void ticket_unlock(ticket_lock_t *lock);

void mcs_lock_init();
void mcs_lock(mcs_node_t *my_node);
void mcs_unlock(mcs_node_t *my_node);

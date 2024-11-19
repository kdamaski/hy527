/*
 * Atomic operations for 4-byte values for the cluster nodes
 * and in gcc inline assembly.
 */

typedef struct adt_lock *lock_t;

void lock_init(void *lock);

void lock(void *lock);

void unlock(void *lock);

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
  asm volatile("xchgq %0, %1"
               : "=r"(old_value), "+m"(*(unsigned long *)ptr)
               : "0"(new_value)

               : "memory");
  return old_value;
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

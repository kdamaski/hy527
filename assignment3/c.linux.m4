define(KOSTAS_ENV, 
`
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include "atomic.h"

#define MAX_THREADS 8

struct timeval the_time;
int thread_count = 0;
int id = 0;
barrier_t barrier;
pthread_t threads[MAX_THREADS];

spinlock_t s_lock;

void spinlock_init() { s_lock.lock = 0; }

void spinlock_lock(spinlock_t *lock) {
  while (1) {
    if (CMPXCHG(&lock->lock, 0, 1) == 0) {
      return;
    }
    __asm__ volatile("pause");
  }
}
void spinlock_unlock(spinlock_t *lock) { DECREMENT(lock->lock); }

ticket_lock_t t_lock;

void ticket_lock_init(ticket_lock_t *lock) {
  lock->next_ticket = 0;
  lock->curr_ticket = 0;
}

void ticket_lock(ticket_lock_t *lock) {
  unsigned my_ticket;
  FETCH_AND_INCREMENT(lock->next_ticket,
                      my_ticket);

  while (lock->curr_ticket != my_ticket) {
    __asm__ volatile("pause");
  }
}

void ticket_unlock(ticket_lock_t *lock) {
  INCREMENT(lock->curr_ticket);
}

mcs_lock_t mcslock;

void mcs_lock_init() { mcslock.tail = NULL; }

void mcs_lock(mcs_node_t *my_node) {

  my_node->next = NULL;
  my_node->locked = 1;

  mcs_node_t *prev =
      (mcs_node_t *)EXCHANGE(&mcslock.tail, (unsigned long)my_node);

  if (prev != NULL) {
    STORE_EXPLICIT(&prev->next, (unsigned long)my_node);

    while (my_node->locked) {
      ;
    }
  }
}

void mcs_unlock(mcs_node_t *my_node) {
  mcs_node_t *successor = (mcs_node_t *)LOAD_EXPLICIT(&my_node->next);
  if (successor == NULL) {
    if (CMPXCHG(&mcslock.tail, (unsigned long)my_node, (unsigned long)NULL) ==
        (unsigned long)my_node) {
      return;
    }
    while ((successor = (mcs_node_t *)LOAD_EXPLICIT(&my_node->next)) == NULL)
      ;
  }
  STORE_EXPLICIT(&successor->locked, 0);
}

void barrier_init(barrier_t *barrier, int num_threads) {
  barrier->thr_count = 0;
  barrier->bar_phase = 0;
  barrier->num_threads = num_threads;
}

void barrier_wait(barrier_t *barrier) {
  int old_value;
  int local_phase = barrier->bar_phase;

  FETCH_AND_INCREMENT(barrier->thr_count, old_value);

  if (barrier->thr_count == barrier->num_threads) {
    barrier->thr_count = 0;
    MEMORY_BARRIER();
    barrier->bar_phase++;
  } else {
    while (barrier->bar_phase == local_phase) {
      __asm__ volatile("pause");
    }
  }
}
')

define(K_LOCKINIT, `mcs_lock_init();')

define(KOSTAS_INITENV, `
K_LOCKINIT();
')


define(G_MALLOC, `malloc($1);')
define(MALLOC, `malloc($1);')

define(EXTERN_ENV,
`
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>

#define MAX_THREADS 8

typedef struct{
 int needed;
 int called;
 pthread_mutex_t mutex;
 pthread_cond_t cond;
} barrier_t;

extern void barrier_init(barrier_t *);
extern void barrier_wait(barrier_t *, int);
extern struct timeval the_time;
extern pthread_cond_t barrier_cond;
extern pthread_mutex_t barrier_mutex;
extern int thread_count;
extern pthread_t threads[MAX_THREADS];
extern int id;
extern LOCKDEC(id_lock);
')

define(MAIN_ENV, 
`
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>

#define MAX_THREADS 8

typedef struct{
 int needed;
 int called;
 pthread_mutex_t mutex;
 pthread_cond_t cond;
} barrier_t;

void barrier_init(barrier_t *barrier)
{
    barrier->needed = -1;
    barrier->called = 0;
    pthread_mutex_init(&barrier->mutex,NULL);
    pthread_cond_init(&barrier->cond,NULL);
}

void barrier_wait(barrier_t *barrier, int cpu_no)
{
    pthread_mutex_lock(&barrier->mutex);
    if (barrier->needed == -1) barrier->needed = cpu_no;
    if (barrier->needed != cpu_no) {printf("Error in appl\n");}
    barrier->called++;
    if (barrier->called == barrier->needed) {
        barrier->called = 0;
        barrier->needed = -1;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        pthread_cond_wait(&barrier->cond,&barrier->mutex);
    }
    pthread_mutex_unlock(&barrier->mutex);
}

struct timeval the_time;
pthread_cond_t barrier_cond;
pthread_mutex_t barrier_mutex;
int thread_count = 0;
pthread_t threads[MAX_THREADS];
int id = 0;
LOCKDEC(id_lock);
')

define(MAIN_INITENV, `
LOCKINIT(id_lock);
pthread_mutex_init(&barrier_mutex, NULL);
pthread_cond_init(&barrier_cond, NULL);
pthread_mutex_lock(&barrier_mutex);
pthread_mutex_unlock(&barrier_mutex);
')
define(MAIN_END, `{int ret; pthread_exit(&ret);}')

define(CREATE, `
pthread_create(&threads[thread_count],NULL,$1,NULL);
thread_count++;
')

define(BARDEC, `barrier_t $1;')
define(BARINIT, `barrier_init(&$1, $2);')
define(BARRIER, `barrier_wait(&$1);')

define(LOCKDEC,  `pthread_mutex_t $1;')
define(LOCKINIT, `pthread_mutex_init((pthread_mutex_t*)&($1), NULL);')
define(LOCK,     `
pthread_mutex_lock((pthread_mutex_t*)&($1));
')
define(UNLOCK,   `
pthread_mutex_unlock((pthread_mutex_t*)&($1));
')

define(ACQUIRE,  `pthread_acquire((pthread_mutex_t*)&($1));')
define(RELEASE,  `pthread_release((pthread_mutex_t*)&($1));')

define(ALOCKDEC,  `pthread_mutex_t ($1[$2]);')
define(ALOCKINIT, `{
                      int loop_j;
                      for(loop_j=0; loop_j < $2; loop_j++){
                          pthread_mutex_init((pthread_mutex_t*)&($1[loop_j]), NULL);
                      }
                   }')
define(ALOCK,      `pthread_mutex_lock((pthread_mutex_t*)&($1[$2]));')
define(AULOCK,     `pthread_mutex_unlock((pthread_mutex_t*)&($1[$2]));')

define(AACQUIRE,   `pthread_acquire((pthread_mutex_t*)&($1[$2]));')
define(ARELEASE,   `pthread_release((pthread_mutex_t*)&($1[$2]));')

define(WAIT_FOR_END, `')
define(CLOCK, `{
                  unsigned long now;
                  gettimeofday(&the_time, NULL);
                  now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
                  $1 = (unsigned int)now;
               }')

define(RESET_STATISTICS, `')
define(GET_PID, `{
	LOCK(id_lock);
	$1 = id++;
	UNLOCK(id_lock);
}')
define(AUG_ON, `')
define(AUG_OFF, `')
define(ALLOCATE_PAGE, `')
define(ALLOCATE_RANGE, `')

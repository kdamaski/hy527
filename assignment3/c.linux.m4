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
#include "atomic.h"

#define MAX_THREADS 8

struct timeval the_time;
int thread_count = 0;
int id = 0;

typedef struct {
  volatile unsigned thr_count;
  volatile unsigned bar_phase;
} barrier_t;

barrier_t barrier;

void barrier_init(barrier_t *barrier) {
  barrier->thr_count = 0;
  barrier->bar_phase = 0;
}

void barrier_wait(barrier_t *barrier, int num_threads) {
  int old_value;
  int local_phase = barrier->bar_phase;

  FETCH_AND_INCREMENT(barrier->thr_count, old_value);

  if (barrier->thr_count == num_threads) {
    barrier->thr_count = 0;
    MEMORY_BARRIER();
    barrier->bar_phase++;
  } else {
    while (barrier->bar_phase == local_phase) {
      __asm__ volatile("pause");
    }
  }
}

pthread_t threads[MAX_THREADS];

LOCKDEC(id_lock)
')

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
#include "atomic.h"

#define MAX_THREADS 8

typedef struct {
  volatile unsigned thr_count;
  volatile unsigned bar_phase;
} barrier_t;

extern void barrier_init(barrier_t *);
extern void barrier_wait(barrier_t *, int);

extern struct timeval the_time;

extern int thread_count;
extern pthread_t threads[MAX_THREADS];
extern int id;

extern LOCKDEC(id_lock);
')

define(LOCKDEC,  `lock_t $1;')
define(LOCKINIT, `lock_init (&$1);')
define(LOCK,     `
lock(&($1));
')
define(UNLOCK,   `
unlock(&($1));
')
define(MAIN_INITENV, `
LOCKINIT(id_lock)
')


define(G_MALLOC, `malloc($1);')
define(MALLOC, `malloc($1);')

define(MAIN_END, `{int ret; pthread_exit(&ret);}')

define(CREATE, `
pthread_create(&threads[thread_count],NULL,$1,NULL);
thread_count++;
')

define(BARDEC, `barrier_t $1;')
define(BARINIT, `barrier_init(&$1);')
define(BARRIER, `barrier_wait(&$1, $2);')

define(ACQUIRE,  `lock(&($1));')
define(RELEASE,  `unlock(&($1));')

define(ALOCKDEC,  `lock_t ($1[$2]);')
define(ALOCKINIT, `{
                      int loop_j;
                      for(loop_j=0; loop_j < $2; loop_j++){
                          lock_init((lock_t*)&($1[loop_j]));
                      }
                   }')
define(ALOCK,      `lock(&($1[$2]));')
define(AULOCK,     `unlock(&($1[$2]));')

define(AACQUIRE,   `lock(&($1[$2]));')
define(ARELEASE,   `unlock(&($1[$2]));')

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

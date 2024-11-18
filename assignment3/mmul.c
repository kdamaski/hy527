#include "atomic.h"
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

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
  FETCH_AND_INCREMENT(lock->next_ticket, my_ticket);

  while (lock->curr_ticket != my_ticket) {
    __asm__ volatile("pause");
  }
}

void ticket_unlock(ticket_lock_t *lock) { INCREMENT(lock->curr_ticket); }

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

#include <assert.h>
#define DEFAULT_N 2000
#define DEFAULT_P 1

struct thread_data {
  unsigned tid;
  unsigned t_first_i;
  unsigned t_last_i;
};

int P = DEFAULT_P;
int N = DEFAULT_N;

unsigned start, finish;

void init_matrices(char *fname, double A[N * N], double B[N * N], int dim);

void *matrix_multiplication(void *);

barrier_t mul_barrier;

unsigned array_size;
unsigned elems_per_thread;

double A[DEFAULT_N * DEFAULT_N], B[DEFAULT_N * DEFAULT_N],
    C[DEFAULT_N * DEFAULT_N];

int main(int argc, char *argv[]) {

  mcs_lock_init();
  ;
  ;

  int c;
  while ((c = getopt(argc, argv, "p:n:")) != -1) {
    switch (c) {
    case 'n':
      N = atoi(optarg);
      if (N < 10 || N > DEFAULT_N) {
        fprintf(stderr, "N must be >= 10 and <= DEFAULT_N\n");
        exit(-1);
      }
      break;
    case 'p':
      P = atoi(optarg);
      if (P < 1 || P > MAX_THREADS) {
        fprintf(stderr, "P must be >= 1 and <= 8\n");
        exit(-1);
      }
      break;
    default:
      break;
    }
  }

  array_size = N * N;
  elems_per_thread = array_size / P;

  // assert((elems_per_thread % P) == 0); // assume there is no remainder

  barrier_init(&mul_barrier, P);
  init_matrices("matrixfile", A, B, N);

  struct thread_data thread_args[P - 1];

  unsigned remainder = array_size % P;

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    start = (unsigned int)now;
  }
  for (int i = 1; i < P; i++) {
    thread_args[i].tid = i;
    thread_args[i].t_first_i = i * elems_per_thread;
    thread_args[i].t_last_i = ((i + 1) * elems_per_thread);
    if (i == P - 1) {
      thread_args[i].t_last_i += remainder;
    }
    pthread_create(&threads[i], NULL, matrix_multiplication, &thread_args[i]);
  }

  // assign work to the main process
  unsigned starttime, finishtime;
  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    starttime = (unsigned int)now;
  }
  for (int i = 0; i < elems_per_thread; ++i) {
    C[i] = A[i] * B[i];
  }
  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    finishtime = (unsigned int)now;
  }
  printf("main Thread spent time of %u sec\n", finishtime - starttime);

  barrier_wait(&mul_barrier);
  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    finish = (unsigned int)now;
  }
  printf("Clock count total time of %u for the whole program\n",
         finish - start);

  // print C for small N
  FILE *outf = fopen("matrix_multiplied", "w");
  assert(outf);
  printf("Writing output to file: matrix_multiplied\n");
  for (int i = 0; i < array_size; ++i) {
    fprintf(outf, "%lf\n", C[i]);
  }
  fclose(outf);
  //
  return 0;
}

void init_matrices(char *fname, double A[N * N], double B[N * N], int dim) {
  assert(fname);
  FILE *f = fopen(fname, "r");
  assert(f);
  for (unsigned i = 0; i < array_size; ++i) {
    fscanf(f, "%lf\n", &A[i]);
  }
  for (unsigned i = 0; i < array_size; ++i) {
    fscanf(f, "%lf\n", &B[i]);
  }
  fclose(f);
}

void *matrix_multiplication(void *args) {
  unsigned starttime, finishtime;

  struct thread_data curr_thr = *(struct thread_data *)args;

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    starttime = (unsigned int)now;
  }
  for (unsigned i = curr_thr.t_first_i; i < curr_thr.t_last_i; ++i) {
    C[i] = A[i] * B[i];
  }
  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    finishtime = (unsigned int)now;
  }
  printf("Thread %u spent total time %u\n", curr_thr.tid,
         finishtime - starttime);
  barrier_wait(&mul_barrier);
  {
    int ret;
    pthread_exit(&ret);
  }
}

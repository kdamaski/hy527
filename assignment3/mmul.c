#include "atomic.h"
#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 8

extern void barrier_init(barrier_t *barrier, int num_threads);
extern void barrier_wait(barrier_t *barrier);

struct timeval the_time;
pthread_cond_t barrier_cond;
pthread_mutex_t barrier_mutex;
int thread_count = 0;
pthread_t threads[MAX_THREADS];
int id = 0;
/* standard libraries structs, functions, macros (e.g MAX_THREADS 8) */
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

void init_matrices(char *fname, double A[N * N], double B[N * N]);

void *matrix_multiplication(void *);

barrier_t mul_barrier;

unsigned array_size;
unsigned elems_per_thread;

double A[DEFAULT_N * DEFAULT_N], B[DEFAULT_N * DEFAULT_N],
    C[DEFAULT_N * DEFAULT_N];

int main(int argc, char *argv[]) {

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
  init_matrices("matrixfile", A, B);

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

void init_matrices(char *fname, double A[N * N], double B[N * N]) {
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
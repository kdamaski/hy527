#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_N 2000
#define DEFAULT_P 1

MPI_Status status;

int P = DEFAULT_P;
int N = DEFAULT_N;

unsigned start, finish;

void init_matrices(char *fname, double A[N * N], double B[N * N], int dim);

void *matrix_multiplication(void *);

unsigned array_size;
unsigned elems_per_thread;

double A[DEFAULT_N * DEFAULT_N], B[DEFAULT_N * DEFAULT_N],
    C[DEFAULT_N * DEFAULT_N];

struct timeval the_time;

int main(int argc, char *argv[]) {

  int nprocs, mynum, info, source, dest;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mynum);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  array_size = N * N;
  elems_per_thread = array_size / P;

  // assert((elems_per_thread % P) == 0); // assume there is no remainder

  init_matrices("matrixfile", A, B, N);

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    start = (unsigned int)now;
  }
  unsigned remainder = array_size % P;

  // thread_args[i].tid = i;
  // thread_args[i].t_first_i = i * elems_per_thread;
  // thread_args[i].t_last_i = ((i + 1) * elems_per_thread) - 1;
  // if (i == P - 1) {
  //   thread_args[i].t_last_i += remainder;
  // }
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

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    starttime = (unsigned int)now;
  }
  // for (unsigned i = curr_thr.t_first_i; i < curr_thr.t_last_i; ++i) {
  //   C[i] = A[i] * B[i];
  // }
  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    finishtime = (unsigned int)now;
  }
  // printf("Thread %u spent total time %u\n", curr_thr.tid,
  //        finishtime - starttime);
}

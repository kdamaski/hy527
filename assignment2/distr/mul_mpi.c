#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_N 1000
#define DEFAULT_P 1

int N = DEFAULT_N;

unsigned start, finish;

void init_matrices(char *fname, double A[N * N], double B[N * N], int dim);

void *matrix_multiplication(void *);

unsigned array_size;
unsigned chunk_size;

struct timeval the_time;

int main(int argc, char *argv[]) {

  double *A, *B, *C;
  int nprocs, mynum, info, source, dest;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mynum);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  array_size = N * N;
  chunk_size = array_size / nprocs;

  // Allocate memory for each process's chunk
  double *A_local = malloc(chunk_size * sizeof(double));
  double *B_local = malloc(chunk_size * sizeof(double));
  double *C_local = malloc(chunk_size * sizeof(double));

  if (mynum == 0) {
    A = malloc(array_size * sizeof(double));
    B = malloc(array_size * sizeof(double));
    C = malloc(array_size * sizeof(double));
    init_matrices("matrixfile", A, B, N);
  }

  // since MPI_Scatter and MPI_Gather exist we do not need indexes
  // unsigned myfirst, mylast;
  // myfirst = mynum * chunk_size;
  // mylast = ((mynum + 1) * chunk_size) - 1;
  // unsigned remainder = array_size % nprocs;
  // we assume that array_size % nprocs is 0
  // if (mynum == nprocs - 1) {
  //   mylast += remainder;
  // }

  MPI_Scatter(A, chunk_size, MPI_DOUBLE, A_local, chunk_size, MPI_DOUBLE, 0,
              MPI_COMM_WORLD);
  MPI_Scatter(B, chunk_size, MPI_DOUBLE, B_local, chunk_size, MPI_DOUBLE, 0,
              MPI_COMM_WORLD);

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    start = (unsigned int)now;
  }
  // Each process computes C_local[i] = A_local[i] * B_local[i]
  for (int i = 0; i < chunk_size; i++) {
    C_local[i] = A_local[i] * B_local[i];
  }

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    finish = (unsigned int)now;
  }
  printf("Process %d finished in %u seconds\n", mynum, finish - start);

  // Gather all partial results into the root process
  MPI_Gather(C_local, chunk_size, MPI_DOUBLE, C, chunk_size, MPI_DOUBLE, 0,
             MPI_COMM_WORLD);

  if (mynum == 0) {

    // print C for small N
    FILE *outf = fopen("matrix_multiplied", "w");
    assert(outf);
    printf("Writing output to file: matrix_multiplied\n");
    for (int i = 0; i < array_size; ++i) {
      fprintf(outf, "%lf\n", C[i]);
    }
    fclose(outf);
    // free(A);
    // free(B);
    // free(C);
  }
  // free(A_local);
  // free(B_local);
  // free(C_local);
  MPI_Finalize();
  //
  return 0;
}

void init_matrices(char *fname, double *A, double *B, int dim) {
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

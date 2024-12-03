#include <stdio.h>
#include <stdlib.h>

#define DIM 6500
#define NUM 6

double randfrom(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

/*
 * Assumption that N is a power of processors used is made.
 * This function write 2(N X N) elements on a file, one elemene per line
 */
int create_matrix_file(int N, const char *fname) {
  FILE *mfile = fopen(fname, "w");
  if (!mfile) {
    fprintf(stderr, "Failed to open file %s\n", fname);
    exit(1);
  }
  for (int i = 0; i < 2; ++i) { // 2 N x N arrays
    for (int j = 0; j < N * N; ++j) {
      fprintf(mfile, "%lf\n", randfrom(0.0, 123456.0));
    }
  }
  return 0;
}

int main() {
  char fname[11];
  sprintf(fname, "largefile%d", NUM);
  create_matrix_file(DIM, fname);
  return EXIT_SUCCESS;
}

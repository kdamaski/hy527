#include <stdio.h>
#include <stdlib.h>

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
  create_matrix_file(1000, "matrixfile");
  return EXIT_SUCCESS;
}

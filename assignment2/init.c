#include "init.h"
#include <assert.h>
#include <stdio.h>

void init_matrices(char *fname, double A[N][N], double B[N][N], int dim) {
  assert(fname);
  FILE *f = fopen(fname, "r");
  assert(f);
  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < dim; ++j) {
      fscanf(f, "%lf\n", &A[i][j]);
    }
  }
  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < dim; ++j) {
      fscanf(f, "%lf\n", &B[i][j]);
    }
  }
}
// double A[N][N], B[N][N];

// int main(int argc, char *argv[]) {
//   init_matrices("matrixfile", A, B, N);
//   printf("A[0][0]=%lf\n", A[0][0]);
//   printf("B[50][49]=%lf\n", B[50][49]);
//   return 0;
// }

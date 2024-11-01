#include "init.h"
#include <assert.h>
#include <stdio.h>

void init_matrices(char *fname, double *A, double *B, int N) {
  assert(fname);
  FILE *f = fopen(fname, "r");
  assert(f);
}

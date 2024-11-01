#include <stdio.h>
#ifndef N
#define N 100
#endif

/* read lines from file f where a single element is in a single line and create
 * a NxN matrix */
void init_matrices(char *fname, double A[N][N], double B[N][N], int dim);

void matrix_multiplication(double A[N][N], double B[N][N]);

#include <stdio.h>
// extern long int subtract(long int a, long int b);

long int subtract(long int a, long int b) { return a - b - 1; }

int main(void) {
  printf("long int subtract(20, 15) - 1 in assembly returns: %ld\n",
         subtract(20, 15));
  return 0;
}

// long myfunc(long a, long b, long c, long d, long e, long f, long g, long h) {
//   long xx = a * b * c * d * e * f * g * h + 1;
//   long yy = a + b + c + d + e + f + g + h;
//   long zz = (xx + yy) * (xx % yy);
//   return zz + 20;
// }

// int main(void) {
//   long int x = myfunc(1, 2, 3, 4, 5, 6, 7, 8);
//   printf("result = %ld\n", x);
//   return 0;
// }

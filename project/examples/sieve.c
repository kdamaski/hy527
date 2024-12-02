/* prime number sieve */

#include "chan.h"
#include "thread.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct args {
  unsigned n;
  Chan_T c;
};

void source(struct args *p) {
  int i = 2, limit = p->n;
  Chan_T c = p->c;

  Chan_send(c, &i, sizeof i);
  for (i = 3; i < limit; i += 2)
    Chan_send(c, &i, sizeof i);
  Chan_send(c, &i, 0);
}

void filter(int primes[], Chan_T listen, Chan_T output) {
  int i, j;

  while (Chan_receive(listen, &i, sizeof i) == sizeof i) {
    for (j = 0; primes[j]; j++)
      if (i % primes[j] == 0)
        break;
    if (primes[j] == 0)
      Chan_send(output, &i, sizeof i);
  }
  Chan_send(output, &i, 0);
}

void sink(struct args *p) {
  Chan_T c = p->c;
  int n = p->n, i = 0, j, prime, primes[256];

  primes[0] = 0;
  while (Chan_receive(c, &prime, sizeof prime)) {
    for (j = 0; primes[j]; j++)
      if (prime % primes[j] == 0)
        break;
    if (primes[j] == 0) {
      primes[i++] = prime;
      primes[i] = 0;
      if (i == n) {
        printf("%d:", Thread_self());
        for (j = 0; primes[j]; j++)
          printf(" %d", primes[j]);
        printf("\n");
        p->c = Chan_new(); /* sink(newc, n); */
        Thread_new((int (*)(void *))sink, p, sizeof *p, NULL);
        filter(primes, c, p->c);
        return;
      }
    }
  }
  if (primes[0]) {
    printf("%d:", Thread_self());
    for (j = 0; primes[j]; j++)
      printf(" %d", primes[j]);
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  int limit = argc > 1 ? atoi(argv[1]) : 100;
  int n = argc > 2 ? atoi(argv[2]) : 5;
  struct args args;

  Thread_init();
  args.c = Chan_new(); /* source(c, limit); */
  args.n = limit;
  Thread_new((int (*)(void *))source, &args, sizeof args, NULL);
  args.n = n; /* sink(c, n); */
  Thread_new((int (*)(void *))sink, &args, sizeof args, NULL);
  Thread_exit(0);
  return 0;
}

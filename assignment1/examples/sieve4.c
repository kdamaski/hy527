/* prime number sieve */

#include <stdio.h>
#include <stdlib.h>
#include "chan.h"
#include "thread.h"

struct args {
        Chan_T c; int n, nprime;
};

void source(struct args *p) {
        int i = 2;

        if (Chan_send(p->c, &i, sizeof i))
                for (i = 3; Chan_send(p->c, &i, sizeof i); i += 2)
                        ;
}

void filter(int primes[], Chan_T listen, Chan_T output) {
        int i, j, n = sizeof i;

        while (n) {
                Chan_receive(listen, &i, sizeof i);
                for (j = 0; primes[j]; j++)
                        if (i%primes[j] == 0)
                                break;
                if (primes[j] == 0)
                         n = Chan_send(output, &i, sizeof i);
        }
        Chan_receive(listen, &i, 0);
}

void sink(struct args *p) {
        Chan_T c = p->c;
        int i = 0, j, prime, primes[256];

        primes[0] = 0;
        while (Chan_receive(c, &prime, sizeof prime)) {
                for (j = 0; primes[j]; j++)
                        if (prime%primes[j] == 0)
                                break;
                if (primes[j] == 0) {
                        printf(" %d", prime);
                        primes[i++] = prime;
                        primes[i] = 0;
                        if (--p->nprime <= 0)
                                break;
                        if (i == p->n) {
                                fflush(stdout);
                                p->c = Chan_new();
                                Thread_new((int (*)(void *))sink, p, sizeof *p, NULL);
                                filter(primes, c, p->c);
                                return;
                        }
                }
        }
        printf("\n");
        Chan_receive(c, &prime, 0);
}

int main(int argc, char *argv[]) {
        struct args args;

        Thread_init();
        args.c = Chan_new();
        Thread_new((int (*)(void *))source, &args, sizeof args, NULL);
        args.n      = argc > 2 ? atoi(argv[2]) : 5;
        args.nprime = argc > 1 ? atoi(argv[1]) : 100;
        Thread_new((int (*)(void *))sink,   &args, sizeof args, NULL);
        Thread_exit(0);
        return 0;
}

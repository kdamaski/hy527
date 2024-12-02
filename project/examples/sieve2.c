/* prime number sieve */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "chan.h"
#include "thread.h"
#include "sem.h"

Sem_T mutex;

void print(char *fmt, ...) {
        va_list ap;

        Sem_wait(&mutex);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        Sem_signal(&mutex);
}

struct args {
        unsigned n; Chan_T c; int *np;
};

void source(struct args *p) {
        Chan_T c = p->c; int *np = p->np;
        int i = 2;

        Chan_send(c, &i, sizeof i);
        for (i = 3; *np > 0; i += 2)
                Chan_send(c, &i, sizeof i);
        Chan_send(c, &i, 0);
}

void filter(int primes[], Chan_T listen, Chan_T output) {
        int i, j;

        while (Chan_receive(listen, &i, sizeof i) == sizeof i) {
                for (j = 0; primes[j]; j++)
                        if (i%primes[j] == 0)
                                break;
                if (primes[j] == 0)
                        Chan_send(output, &i, sizeof i);
        }
        Chan_send(output, &i, 0);
}

void sink(struct args *p) {
        unsigned n = p->n; Chan_T c = p->c; int *np = p->np;
        int i = 0, j, prime, primes[256];

        primes[0] = 0;
        while (Chan_receive(c, &prime, sizeof prime)) {
                for (j = 0; primes[j]; j++)
                        if (prime%primes[j] == 0)
                                break;
                if (primes[j] == 0) {
                        if (i == 0)
                                print("%d:", Thread_self());
                        print(" %d", prime);
                        primes[i++] = prime;
                        primes[i] = 0;
                        if (--(*np) > 0 && i == n) {
                                print("\n");
                                p->c = Chan_new();      /* sink(n, newc, np); */
                                Thread_new((int (*)(void *))sink, p, sizeof *p, NULL);
                                filter(primes, c, p->c);
                                break;
                        }
                }
        }
}

int main(int argc, char *argv[]) {
        int nprimes  = argc > 1 ? atoi(argv[1]) : 100;
        int nthreads = argc > 2 ? atoi(argv[2]) : 16;
        struct args args;
        unsigned n = nprimes/nthreads + (nprimes%nthreads != 0);

        Thread_init();
        Sem_init(&mutex, 1);
        args.c = Chan_new();    /* source(c, np); */
        args.np = &nprimes;
        Thread_new((int (*)(void *))source, &args, sizeof args, NULL);
        args.n = n;             /* sink(n, c, np); */
        Thread_new((int (*)(void *))sink,   &args, sizeof args, NULL);
        Thread_exit(0);
        return 0;
}

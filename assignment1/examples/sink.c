#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "chan.h"

struct args { Chan_T c; int limit; };

void source(struct args *p) {
        int i;

        for (i = 0; i < p->limit; i++)
                Chan_send(p->c, &i, sizeof i);
        Chan_send(p->c, &i, 0);
}

void sink(struct args *p) {
        int n;

        while (Chan_receive(p->c, &n, sizeof n) == sizeof n)
                printf(" %d", n);
        printf("\n");
}

int main(int argc, char *argv[]) {
        struct args args;

        Thread_init();
        args.c = Chan_new();
        args.limit = argc > 1 ? atoi(argv[1]) : 100;
        Thread_new((int (*)(void *))source, &args, sizeof args, NULL);
        Thread_new((int (*)(void *))sink,   &args, sizeof args, NULL);
        Thread_exit(0);
        return 0;
}

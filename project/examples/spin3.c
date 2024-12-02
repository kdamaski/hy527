#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "sem.h"

Sem_T mutex;
int y;

struct args {
        Sem_T *mutex;
        int *ip;
};

void incr(struct args *p) {
        int i;
        static int j;

        for (i = 0; i < 10000; i++) {
                Sem_wait(p->mutex);
                *p->ip = *p->ip + 1;
                Sem_signal(p->mutex);
        }
        printf("%d:(%d:y)=%d\n", j, Thread_self(), y);
        j++;
}

int main(int argc, char *argv[]) {
        int i, j;
        int m = argc > 1 ? atoi(argv[1]) : 4;
        int n = argc > 2 ? atoi(argv[2]) : 3;

        Thread_init();
        Sem_init(&mutex, 1);
        for (j = 0; j < n; j++) {
                struct args args;
                args.mutex = &mutex;
                args.ip = &y;
                for (i = 0; i < m; i++)
                        Thread_new((int (*)(void *))incr, &args, sizeof args, NULL);
                Thread_join(0);
                if (y != m*10000)
                        printf("%d:%d\n", j, y);
                y = 0;
        }
        Thread_exit(0);
        return 0;
}

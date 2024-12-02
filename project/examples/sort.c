#include <stdlib.h>
#include <stdio.h>
#include "thread.h"

int cutoff = 10000;

int partition(int a[], int i, int j) {
        int v, k, t;

        j++;
        k = i;
        v = a[k];
        while (i < j) { /* this could otherwise core dump @@@ */
	/*
                i++; while (a[i] < v) i++;
                j--; while (a[j] > v) j--;
	*/
	 	i++; while ((a[i] < v) && (i<=j)) i++;
		j--; while ((a[j] > v) && (i<=j)) j--;
                if (i < j) { t = a[i]; a[i] = a[j]; a[j] = t; }
        }
        t = a[k]; a[k] = a[j]; a[j] = t;
        return j;
}

struct args { int *a, lb, ub; };

void quick(struct args *p) {
        int lb = p->lb, ub = p->ub;
        int k, tid1 = 0, tid2 = 0;

        if (lb >= ub)
                return;
        k = partition(p->a, lb, ub);
        p->lb = lb; p->ub = k - 1;
        if (k - lb > cutoff)
                tid1 = Thread_new((int (*)(void *))quick, p, sizeof *p, NULL);
        else
                quick(p);
        p->lb = k + 1; p->ub = ub;
        if (ub - k > cutoff)
                tid2 = Thread_new((int (*)(void *))quick, p, sizeof *p, NULL);
        else
                quick(p);
        if (tid1 > 0)
                printf("thread %d sorted %d..%d\n", tid1, lb, k - 1);
        if (tid2 > 0)
                printf("thread %d sorted %d..%d\n", tid2, k + 1, ub);
}

int main(int argc, char *argv[]) {
        int i, n = 100000, *x;
        struct args args;

        if (argc >= 2)
                n = atoi(argv[1]);
        if (argc >= 3)
                cutoff = atoi(argv[2]);
        if ((x = malloc(n*sizeof(int))) == NULL) {
                printf("%s: can't allocate x[%d]\n", argv[0], n);
                exit(1);
        }
        x[0] = 314592;
        for (i = 1; i < n; i++)
                if ((x[i] = x[i-1]*271828 % 1000000) < 0)
                        x[i] += 1000000;
        Thread_init();
        args.a = x; args.lb = 0; args.ub = n - 1;
        quick(&args);
        Thread_join(0);
        for (i = 1; i < n; i++)
                if (x[i] < x[i-1]) break;
        printf("%ssorted\n", i != n ? "not " : "");
        if (i != n && n <= 100) {
                for (i = 1; i <= n; i++) {
                        printf("%10d", x[i-1]);
                        if (i%5 == 0)
                                printf("\n");
                }
                if (n%5)
                        printf("\n");
        }
        Thread_exit(i != n);
        return 0;
}

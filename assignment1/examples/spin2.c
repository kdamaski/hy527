#include "sem.h"
#include "thread.h"
#include <stdio.h>
#include <stdlib.h>

Sem_T mutex;
int y = 0;

struct args {
  Sem_T *mutex;
  int *ip;
};

void incr(struct args *p) {
  int i;

  for (i = 0; i < 10000; i++) {
    Sem_wait(p->mutex);
    *p->ip = *p->ip + 1;
    Sem_signal(p->mutex);
  }
}

int main(int argc, char *argv[]) {
  int i, m = argc > 1 ? atoi(argv[1]) : 5;
  static struct args args = {&mutex, &y};

  Thread_init();
  Sem_init(&mutex, 1);
  for (i = 0; i < m; i++)
    Thread_new((int (*)(void *))incr, &args, sizeof args, NULL);
  Thread_join(0);
  printf("%d = %d\n", y, m * 10000);
  Thread_exit(0);
  return 0;
}

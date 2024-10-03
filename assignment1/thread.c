#include "thread.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct thread_queue thr_q;

void Thread_init(void) {
  thr_q.head = thr_q.tail = malloc(sizeof(struct damthread));
  thr_q.next = NULL;
  thr_q.head->id = -1;
  thr_q.head->args = thr_q.head->func = NULL;
}

int Thread_self(void) { return thr_q.head->id; }

void Thread_exit(int code) {
  switch (code) {
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  default:;
  }
}

int Thread_new(int func(void *), void *args, size_t nbytes, unsigned id) {
  struct damthread *new_thr = malloc(sizeof(struct damthread));
  new_thr->func = func;
  new_thr->args = args;
  new_thr->nbytes = nbytes;
  return (new_thr->id = id);
}

int mytestfunc(void *args) { return *(int *)args; }

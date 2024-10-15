#include "thread.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB 1024

unsigned long _STACK;
unsigned n_threads = 0;

extern void _swtch(void *from, void *to);
extern void _thrstart(void *arg, int func(void *));

struct thread_queue *thr_q = NULL;
// struct damthread *curr_thr;

void Thread_init(void) {
  if (thr_q != NULL) {
    fprintf(stderr, "Thread_init has already been called\n");
    return;
  }
  thr_q = (struct thread_queue *)malloc(sizeof(struct thread_queue));
  // allocate the stack size for my user-level threads
  // (unsigned long)malloc(64 * KB)
  posix_memalign((void *)_STACK, 16, 64 * KB);
  if (!thr_q) {
    fprintf(stderr, "malloc failed to allocate space\n");
    exit(1);
  }
  thr_q->head = thr_q->tail = NULL;
}

int Thread_self(void) { return (unsigned)thr_q->head->id; }

// thr_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(int code) {
  switch (code) {
  case 0:
  case 1:
  case 2:
  default:;
    printf("thread %u exited with code %d\n", thr_q->head->id, code);
  }
  thr_dequeue();
}

/* Thread_pause is called by the currently running thread */
void Thread_pause(void) {
  // extract from and to *sp values and call _swtch
  if (thr_q->head->next) {
    void *from = thr_q->head->sp;
    void *to = thr_q->head->next->sp;
    thr_enqueue(thr_dequeue());
    // print_Q();
    _swtch(from, to);
  } else {
    printf("Thread %u cannot pause since there is no other job to run\n",
           thr_q->head->id);
  }
}

int Thread_join(int tid) { return Thread_self(); }

int Thread_new(int func(void *), void *args, size_t nbytes, ...) {
  struct damthread *new_thr =
      (struct damthread *)(_STACK +
                           n_threads * (nbytes + sizeof(struct damthread)));
  // sp is the first element of struct
  new_thr->sp = new_thr + sizeof(struct damthread);
  new_thr->id = (unsigned)new_thr; // Testing in 32 bit architecture
  //
  new_thr->args = (void *)new_thr + sizeof(struct damthread);
  if (!new_thr->args) {
    fprintf(stderr, "malloc failed to allocate space\n");
    return -1;
  }
  memcpy(new_thr->args, args, nbytes);
  new_thr->pc = func;
  thr_enqueue(new_thr); // n_threads adjusted here
  return 0;
}

int mytestfunc(void *args) {
  int arg = *(int *)args;
  printf("Thread %u is running with arg=%d\n", thr_q->head->id, arg);
  Thread_pause();
  printf("Thread %u is back to running with arg=%d\n", thr_q->head->id, arg);
  return Thread_self();
}

int schedule() {
  if (thr_q->head) {
    // _thrstart(thr_q->head->args, thr_q->head->pc);
    // return 0;
    return thr_q->head->pc(thr_q->head->args);
  } else {
    printf("Cannot run Q empty\n");
    return -1;
  }
}

int main() {
  int arg1 = 4, arg2 = 5;
  Thread_init();
  Thread_new(mytestfunc, (void *)&arg1, sizeof(int), 1, NULL);
  Thread_new(mytestfunc, (void *)&arg2, sizeof(int), 2, NULL);
  print_Q();
  Thread_exit(schedule());
  Thread_exit(schedule());
}

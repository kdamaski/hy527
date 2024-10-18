#include "thread.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB 1024

unsigned stack_size;
unsigned n_threads;

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
  if (!thr_q) {
    fprintf(stderr, "malloc failed to allocate space\n");
    exit(1);
  }
  thr_q->head = thr_q->tail = NULL;

  n_threads = 0;
  // allocate the stack size for my user-level threads
  stack_size = 16 * KB;
}

int Thread_new(int func(void *), void *args, size_t nbytes, ...) {
  assert(func && args && (nbytes >= 0));
  struct damthread *new_thr;
  // start of critical section. Will care when preemption comes
  if (posix_memalign((void **)&new_thr, 16,
                     stack_size + sizeof(struct damthread) + nbytes) != 0) {
    fprintf(stderr, "memory allocation failed for thread stack\n");
    exit(1);
  }
  new_thr->pc = func;
  new_thr->args = args;
  // leave space (nbytes) under the stack pointer for args value
  new_thr->sp = (unsigned long *)((void *)new_thr + stack_size +
                                  sizeof(struct damthread));
  // copy the args into the bottom of the allocated stack
  memcpy((void *)new_thr->sp, args, nbytes);

  // Place _thrstart in the return address of our thread stack
  --new_thr->sp;
  *(new_thr->sp) = (unsigned long)_thrstart;
  // now allocate 4 words for the general purpose registers that are saved
  new_thr->sp -= 4;
  new_thr->sp[1] =
      (unsigned long)new_thr->pc; // Those 2 args are for context restore
  new_thr->sp[2] = (unsigned long)args;

  new_thr->id = (unsigned long)new_thr; // Testing in 32 bit architecture
  thr_enqueue(new_thr);
  return 0;
}

int Thread_self(void) { return (unsigned)thr_q->head->id; }

// thr_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(int code) {
  switch (code) {
  case 0:
  case 1:
  case 2:
  default:
    printf("thread %lu exited with code %d\n", thr_q->head->id, code);
  }
  thr_dequeue();
}

/* Thread_pause is called by the currently running thread */
void Thread_pause(void) {
  assert(thr_q->head);
  // extract from and to *sp values and call _swtch
  if (thr_q->head->next) {
    void *from = thr_q->head;
    void *to = thr_q->head->next;
    thr_enqueue(thr_dequeue());
    _swtch(from, to);
  } else {
    printf("Thread %lu cannot pause since there is no other job to run\n",
           thr_q->head->id);
  }
}

int Thread_join(unsigned tid) {
  assert(thr_q->head && thr_q->head->id != tid);
  if (tid != 0) {
    // wait for thread tid
    return tid;
  } else {
    // wait for all threads
    return 0;
  }
}

int mytestfunc(void *args) {
  int arg = *(int *)args;
  printf("Thread %lu is running with arg=%d\n", thr_q->head->id, arg);
  Thread_pause();
  printf("Thread %lu is back to running with arg=%d\n", thr_q->head->id, arg);
  return Thread_self();
}

int schedule() {
  if (thr_q->head) {
    assert(thr_q->head);
    // extract from and to *sp values and call _swtch
    if (thr_q->head->next) {
      thr_q->head->pc(thr_q->head->args);
    } else {
      printf("Thread %lu cannot yield since there is no other job to run\n",
             thr_q->head->id);
    }
    return 0;
  } else {
    printf("Cannot run Q empty\n");
    return -1;
  }
}

int main() {
  int arg1 = 4, arg2 = 5;
  Thread_init();
  Thread_new(mytestfunc, (void *)&arg1, sizeof(int), NULL);
  Thread_new(mytestfunc, (void *)&arg2, sizeof(int), NULL);
  // print_Q();
  schedule();
  // schedule();
}

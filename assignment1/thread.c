#include "thread.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB 1024

unsigned stack_size;
unsigned n_threads;
unsigned long *root_thread;

extern void _swtch(void *from, void *to);
extern void _thrstart(void *arg, int func(void *));

struct thread_queue *thr_q = NULL;
// struct damthread *curr_thr;

struct freelist *free_list = NULL;

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

  // initialize the free list
  free_list = malloc(sizeof(struct freelist));
  if (!free_list) {
    fprintf(stderr, "Malloc failed\n");
    exit(1);
  }
  free_list->head = free_list->next = NULL;
}

char first_thread = 1;
int Thread_new(int func(void *), void *args, size_t nbytes, ...) {
  assert(func && args && (nbytes >= 0));
  struct damthread *new_thr;
  // start of critical section. Will care when preemption comes
  // good time to free the terminated threads before i have a stack smash or sth
  empty_free();
  if (posix_memalign((void **)&new_thr, 16,
                     stack_size + sizeof(struct damthread) + nbytes) != 0) {
    fprintf(stderr, "memory allocation failed for thread stack\n");
    exit(1);
  }
  if (first_thread) { // first thread holds the stack of process
    first_thread = 0;
    root_thread = (unsigned long *)new_thr;
  }

  // those 2 fields seem meaningless but i need them for testing
  new_thr->pc = func;
  new_thr->args = args;
  // init thr_joining list
  new_thr->thr_joining = NULL;
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

  thr_enqueue(new_thr);
  ++n_threads;
  return (new_thr->id = (unsigned long)new_thr);
}

int Thread_self(void) { return (unsigned)thr_q->head->id; }

// thr_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(int code) {
  assert(thr_q->head);
  switch (code) {
  case 0:
  case 1:
  case 2:
  default:
    empty_free();
    // resume threads waiting for current's termination
    struct damthread *tmp = thr_q->head->thr_joining;
    if (tmp) {
      while (tmp) {
        thr_enqueue(tmp);
        tmp = tmp->next;
      }
    }
    //   else if (n_threads == 2) { // n_threads will decrease in thr_dequeue
    //   Thread_pause();
    // }
    if (--n_threads == 0) {
      exit(0);
    } else {
      Thread_pause();
    }
    insert_to_free(thr_dequeue()); // put it in the free list
  }
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
    // last thread to finish!!
    // switch esp to the last thread available
    asm volatile("movl	%0,%%esp\n\t" ::"r"(thr_q->head->sp));

    asm volatile("movl	0(%esp),%ebx\n\t"
                 "movl	4(%esp),%esi\n\t"
                 "movl	8(%esp),%edi\n\t"
                 "movl	12(%esp),%ebp\n\t"
                 "addl	$16, %esp\n\t"
                 "ret\n\t");
  }
}

char join_0_call = 1;
int Thread_join(unsigned tid) { // tid is also the address of the thread
  assert(thr_q->head && thr_q->head->id != tid && join_0_call);
  if (tid != 0) {
    struct damthread *th_to_join = thread_exists(tid);
    if (!th_to_join) {
      return -1; // thread with such tid does not exist in Q
    }
    th_to_join->thr_joining = thr_q->head;
    return tid;
  } else {
    join_0_call = 0;
    // wait for all threads
    Thread_exit(0);
    return 0;
  }
}

int mytestfunc(void *args) {
  int arg = *(int *)args;
  printf("Thread %lu is running with arg=%d\n", thr_q->head->id, arg);
  for (int i = 0; i < 250000; ++i) {
    ++arg;
  }
  printf("Thread %lu spinned 25k times into ++arg=%d\n", thr_q->head->id, arg);
  return Thread_self();
}

char thr_started = 0;
int schedule() { // basically main calls this
  assert(thr_q->head);
  if (thr_started) {
    printf("Already scheduled\n");
    return 1;
  }
  if (thr_q->head) {
    thr_started = 1;
    // start first thread
    thr_q->head->pc(thr_q->head->args); // function call saves the stack of main
    return 0;
  } else {
    printf("Cannot run Q empty\n");
    return -1;
  }
}

int main() {
  int arg1 = 4, arg2 = 5, arg3 = 6;
  Thread_init();
  int thid1 = Thread_new(mytestfunc, (void *)&arg1, sizeof(int), NULL);
  int thid2 = Thread_new(mytestfunc, (void *)&arg2, sizeof(int), NULL);
  int thid3 = Thread_new(mytestfunc, (void *)&arg3, sizeof(int), NULL);
  schedule();
  Thread_join(0);
}

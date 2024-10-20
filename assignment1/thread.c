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

struct th_ready_q *ready_q = NULL;

struct u_thread *join0_thread = NULL; // Running thread. Do not need it

struct u_thread *free_list = NULL;

struct u_thread *join_list = NULL;

void Thread_init(void) {
  if (ready_q != NULL) {
    fprintf(stderr, "Thread_init has already been called\n");
    return;
  }
  ready_q = (struct th_ready_q *)malloc(sizeof(struct th_ready_q));
  assert(ready_q);
  ready_q->head = ready_q->tail = NULL;

  n_threads = 0;
  // allocate the stack size for my user-level threads
  stack_size = 16 * KB;
  free_list = malloc(sizeof(struct u_thread));
  // guard node for free list
  free_list->id = -1;
  join_list = malloc(sizeof(struct u_thread));
  assert(join_list);
  join_list->id = -1;
}

int Thread_new(int func(void *), void *args, size_t nbytes, ...) {
  assert(func && args && (nbytes >= 0));
  struct u_thread *new_thr;
  // start of critical section. Will care when preemption comes
  // good time to free the terminated threads before i have a stack smash or sth
  empty_free();
  if (posix_memalign((void **)&new_thr, 16,
                     stack_size + sizeof(struct u_thread) + nbytes) != 0) {
    fprintf(stderr, "memory allocation failed for thread stack\n");
    exit(1);
  }
  // those 2 fields seem meaningless but i need them for testing
  new_thr->pc = func;
  new_thr->args = args;
  // leave space (nbytes) under the stack pointer for args value
  new_thr->sp =
      (unsigned long *)((void *)new_thr + stack_size + sizeof(struct u_thread));
  // copy the args into the bottom of the allocated stack
  memcpy((void *)new_thr->sp, args, nbytes);

  // Place _thrstart in the return address of our thread stack
  --new_thr->sp;
  *(new_thr->sp) = (unsigned long)_thrstart;
  // now allocate 4 words for the general purpose registers that are saved
  new_thr->sp -= 4;
  new_thr->sp[1] = (unsigned long)func; // Those 2 args are for context restore
  new_thr->sp[2] = (unsigned long)args; // in the order of the given swtch.s

  // init queue state
  new_thr->handle = 1; // mark thread initialized
  new_thr->next = new_thr->jlist = NULL;
  ready_enqueue(new_thr);
  ++n_threads;

  return (new_thr->id = (unsigned long)new_thr);
}

int Thread_self(void) { return (unsigned)ready_q->head->id; }

// ready_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(int code) {
  assert(ready_q->head);
  switch (code) {
  case 0:
  case 1:
  case 2:
  default:
    empty_free();
    // resume threads waiting for current's termination
    struct u_thread *tmp = ready_q->head->jlist;
    if (tmp) {
      while (tmp) {
        ready_enqueue(tmp);
        tmp = tmp->next;
      }
    }
    if (--n_threads == 0) {
      exit(0);
    } else {
      Thread_pause();
    }
    push_to_free(ready_dequeue()); // put it in the free list
  }
}

/* Thread_pause is called by the currently running thread */
void Thread_pause(void) {
  assert(ready_q->head);
  // extract from and to *sp values and call _swtch
  if (ready_q->head) {
    void *from = ready_q->head;
    void *to = ready_q->head = ready_q->head->next;
    // ready_enqueue(ready_dequeue());
    _swtch(from, to);
  } else {
    // last thread to finish!!
    // switch esp to the last thread available
    // asm volatile("movl	%0,%%esp\n\t" ::"r"(ready_q->head->sp));

    // asm volatile("movl	0(%esp),%ebx\n\t"
    //              "movl	4(%esp),%esi\n\t"
    //              "movl	8(%esp),%edi\n\t"
    //              "movl	12(%esp),%ebp\n\t"
    //              "addl	$16, %esp\n\t"
    //              "ret\n\t");
  }
}

char join_0_called = 0;

int Thread_join(unsigned long tid) {
  assert(ready_q->head && ready_q->head->id != tid && !join_0_called);
  struct u_thread *th_to_join = (struct u_thread *)tid;
  if (tid != 0) {                  // tid is also the address of the thread
    if (th_to_join->handle == 0) { // if it is put in free list
      return -1;
    } // NOTE th to join did not get removed from ready_q
    ready_q->head->jlist = th_to_join;
    push_to_join(th_to_join);
    Thread_pause();
    return tid;
  } else {
    join_0_called = 1;
    // wait for all threads
    if (n_threads > 1) {
      join0_thread = ready_q->head;
      Thread_pause();
    }
    return 0;
  }
}

int mytestfunc(void *args) {
  int arg = *(int *)args;
  printf("Thread %lu is running with arg=%d\n", ready_q->head->id, arg);
  for (int i = 0; i < 250000; ++i) {
    ++arg;
  }
  printf("Thread %lu spinned 25k times into ++arg=%d\n", ready_q->head->id,
         arg);
  return Thread_self();
}

int myjointestfunc(void *args) {
  int arg = *(int *)args;
  int arg1 = arg + 1000;
  printf("Thread %lu is running with arg=%d and will create a thread with "
         "arg=%d and join it\n",
         ready_q->head->id, arg, arg1);
  Thread_join(Thread_new(mytestfunc, (void *)(&arg1), sizeof(int), NULL));
  printf("Thread %lu returned after calling join on thread with arg=%d\n",
         ready_q->head->id, arg1);
  return Thread_self();
}

// char thr_started = 0;
// int schedule() { // basically main calls this
//   assert(ready_q->head);
//   if (thr_started) {
//     printf("Already scheduled\n");
//     return 1;
//   }
//   if (ready_q->head) {
//     thr_started = 1;
//     // start first thread
//     ready_q->head = ready_q->head;
//     ready_q->head->pc(
//         ready_q->head->args); // function call saves the stack of main
//     return 0;
//   } else {
//     printf("Cannot run empty Q\n");
//     return -1;
//   }
// }

int main() {
  int arg1 = 4, arg2 = 5, arg3 = 6;
  Thread_init();
  int thid1 = Thread_new(mytestfunc, (void *)&arg1, sizeof(int), NULL);
  int thid2 = Thread_new(mytestfunc, (void *)&arg2, sizeof(int), NULL);
  int thid3 = Thread_new(mytestfunc, (void *)&arg3, sizeof(int), NULL);
  Thread_exit(0); // runs the first thread
  Thread_join(thid2);
  printf("Yeah returned\n");
}

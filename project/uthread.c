#include "uthread.h"
#include "misc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB 1024

extern void _swtch(void *from, void *to);
extern void _thrstart(void *arg, int func(void *));

struct uthread_queue *Thread_init(void) {
  struct uthread_queue *uq =
      (struct uthread_queue *)malloc(sizeof(struct uthread_queue));
  assert(uq);
  uq->ready_q = (struct th_ready_q *)malloc(sizeof(struct th_ready_q));
  assert(uq->ready_q);
  uq->ready_q->head = uq->ready_q->tail = uq->free_list = uq->join0_thread =
      NULL;
  // allocate the stack size for my user-level threads
  uq->stack_size = 4 * KB;

  uq->main_thr = (struct u_thread *)malloc(sizeof(struct u_thread));
  assert(uq->main_thr);
  uq->main_thr->id = -1;

  uq->curr_thr = uq->main_thr;
  uq->n_threads = 1;
  return uq;
}

unsigned long Thread_new(void *func(void *), void *args, long nbytes,
                         struct uthread_queue *uq) {
  assert(func && (nbytes >= 0));
  struct u_thread *new_thr = NULL;
  // start of critical section. Will care when preemption comes
  // good time to free the terminated threads before i have a stack smash or sth
  empty_free(uq->free_list);
  if (posix_memalign((void **)&new_thr, 16,
                     (uq->stack_size + sizeof(struct u_thread) + nbytes + 15) &
                         ~15) != 0) {
    fprintf(stderr, "memory allocation failed for thread stack\n");
    exit(1);
  }

  // unsigned total_stack_sz = // syntax below says address must be aligned 16
  //     (uq->stack_size + sizeof(struct u_thread) + nbytes + 15) & ~15;
  // new_thr = (struct u_thread *)calloc(1, total_stack_sz);
  // assert(new_thr);

  // nbytes above stack length
  new_thr->sp = (unsigned long *)((void *)new_thr + uq->stack_size +
                                  sizeof(struct u_thread));

  // in the bottom place the args
  void *my_args = new_thr->sp;
  if (args) {
    memcpy(new_thr->sp, args, nbytes);
  }
  // while (((unsigned long)new_thr->sp) & 15)
  //   new_thr->sp--;

  // Place _thrstart in the return address of our thread stack
  --new_thr->sp;
  *(new_thr->sp) = (unsigned long)_thrstart;

  // new_thr->sp -= 24;

  // new_thr->sp[17] = (unsigned long)func; // Those 2 args are for context
  // restore
  // // copy the args into the bottom of the allocated stack
  // new_thr->sp[18] = (unsigned long)my_args;
  new_thr->sp -= 10;

  new_thr->sp[1] = (unsigned long)func; // Those 2 args are for context restore
  // copy the args into the bottom of the allocated stack
  new_thr->sp[2] = (unsigned long)my_args;

  // init queue state
  new_thr->to_run = 1; // mark thread initialized
  new_thr->next = new_thr->jlist = NULL;

  // if Join called thread_new then do not add thread to ready_q. Not
  ready_enqueue(new_thr, uq->ready_q);
  ++uq->n_threads;

  return (new_thr->id = (unsigned long)new_thr);
}

int Thread_self(struct u_thread curr_thr) { return (unsigned)curr_thr.id; }

// ready_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(void *args) {
  struct uthread_queue *uq = (struct uthread_queue *)args;
  assert(uq->curr_thr);
  empty_free(uq->free_list);
  // resume threads waiting for current's termination
  struct u_thread *tmp = uq->curr_thr->jlist;
  if (tmp) {
    while (tmp) { // enQ threads waiting on me
      ready_enqueue(tmp, uq->ready_q);
      tmp = tmp->next;
      // ++n_threads;
    }
  }
  --uq->n_threads;
  if (uq->n_threads == 0) {
    printf("n_threads == 0. Exiting\n");
    exit(0);
  } else {
    // printf("Thread %lu is exiting\n", uq->curr_thr->id);
    void *from = uq->curr_thr;
    push_to_free(uq->curr_thr, uq->free_list);
    uq->curr_thr = ready_dequeue(uq->ready_q);
    if (uq->curr_thr == uq->main_thr) {
      ready_enqueue(uq->curr_thr, uq->ready_q);
      uq->curr_thr = ready_dequeue(uq->ready_q);
      // assert(uq->curr_thr != uq->main_thr);
    }
    _swtch(from, uq->curr_thr);
  }
}

/* Thread_pause is called by the currently running thread */
void Thread_pause(struct uthread_queue *uq) {
  assert(uq->curr_thr);
  // assert(n_threads > 1);
  // TODO temp hack to ignore swaping to main thread
  if (uq->curr_thr->id != -1) { // if not main
    ready_enqueue(uq->curr_thr, uq->ready_q);
  } else {
    uq->n_threads--;
  }
  void *from = uq->curr_thr;
  void *to = uq->curr_thr = ready_dequeue(uq->ready_q);
  _swtch(from, to);
}

char join_0_called = 0;

int Thread_join(unsigned long tid, struct uthread_queue *uq) {
  // assert(curr_thr && curr_thr->id != tid && !join_0_called);
  if (tid != 0) { // tid is also the address of the thread
    struct u_thread *th_to_join = (struct u_thread *)tid;
    if (th_to_join->to_run == 0) { // if it is put in free list
      printf("Thread with tid %lu has already exited\n", tid);
      return -1;
    }

    void *from = uq->curr_thr;
    push_to_join(th_to_join,
                 uq->curr_thr); // pushes uq->curr_thr into tid's wait_Q
    uq->curr_thr = ready_dequeue(uq->ready_q);
    if (uq->curr_thr == uq->main_thr) {
      ready_enqueue(uq->curr_thr, uq->ready_q);
      uq->curr_thr = ready_dequeue(uq->ready_q);
      // assert(uq->curr_thr != main_thr);
    }
    _swtch(from, uq->curr_thr);

    return tid;
  } else {
    join_0_called = 1;

    if (uq->n_threads > 1) {
      uq->join0_thread = uq->curr_thr;
      Thread_pause(uq);
    }
    return 0;
  }
}

void *mytestfunc(void *args) {
  struct uthread_queue *uq = (struct uthread_queue *)args;
  print_Q(uq);
  Thread_pause(uq);
  return args;
}

int myjointestfunc(void *args) {
  struct uthread_queue *uq = (struct uthread_queue *)args;

  Thread_join(
      Thread_new(mytestfunc, (void *)(uq), sizeof(struct uthread_queue), uq),
      uq);

  return Thread_self(*uq->curr_thr);
}

// int main() {
//   struct uthread_queue *uq = Thread_init();
//   int arg1 = 1, arg2 = 2, arg3 = 3, arg4 = 4;
//   unsigned long thid1 =
//       Thread_new(mytestfunc, (void *)uq, sizeof(uthread_queue), uq);
//   unsigned long thid2 =
//       Thread_new(mytestfunc, (void *)uq, sizeof(uthread_queue), uq);
//   unsigned long thid3 =
//       Thread_new(mytestfunc, (void *)uq, sizeof(uthread_queue), uq);
//   // unsigned long thid3 = Thread_new(myjointestfunc, (void *)&uq,
//   //                                  sizeof(struct uthread_queue *), uq);

//   Thread_join(0, uq);
//   // Thread_pause();
//   printf("Returned to main successfully\n");
// }

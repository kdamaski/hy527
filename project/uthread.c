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
  // if (uq != NULL) {
  //   fprintf(stderr, "Thread_init has already been called\n");
  //   return;
  // }

  struct uthread_queue *uq =
      (struct uthread_queue *)malloc(sizeof(struct uthread_queue));
  assert(uq);
  uq->ready_q = (struct th_ready_q *)malloc(sizeof(struct th_ready_q));
  assert(uq->ready_q);
  uq->ready_q->head = uq->ready_q->tail = NULL;
  // allocate the stack size for my user-level threads
  uq->stack_size = 4 * KB;

  uq->main_thr = malloc(sizeof(struct u_thread));
  uq->main_thr->id = -1;

  uq->curr_thr = uq->main_thr;
  uq->n_threads = 1;
  return uq;
}

unsigned long Thread_new(int func(void *), void *args, long nbytes,
                         struct uthread_queue *uq) {
  assert(func && args && (nbytes >= 0) && uq);
  struct u_thread *new_thr = NULL;
  // start of critical section. Will care when preemption comes
  // good time to free the terminated threads before i have a stack smash or sth
  empty_free(uq->free_list);
  if (posix_memalign((void **)&new_thr, 16,
                     uq->stack_size + sizeof(struct u_thread) + nbytes + 4) !=
      0) {
    fprintf(stderr, "memory allocation failed for thread stack\n");
    exit(1);
  }

  // nbytes above stack length
  new_thr->sp = (unsigned long *)((void *)new_thr + uq->stack_size +
                                  sizeof(struct u_thread));

  // in the bottom place the args
  memcpy((void *)new_thr->sp, args, nbytes);
  //
  void *my_args = new_thr->sp;
  // Place _thrstart in the return address of our thread stack
  --new_thr->sp;
  *(new_thr->sp) = (unsigned long)_thrstart;
  // now allocate 4 words for the general purpose registers that are saved
  new_thr->sp -= 4;
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

int Thread_self(struct u_thread *curr_thr) { return (unsigned)curr_thr->id; }

// ready_enqueue assumes that the Thread_init has been called
// Thread_exit is called by __thrstart routine in assmbly
void Thread_exit(int code, struct uthread_queue *uq) {
  assert(uq->curr_thr);
  switch (code) {
  case 0:
  case 1:
  case 2:
  default:
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
      printf("Thread %lu is exiting with code %d\n", uq->curr_thr->id, code);
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
}

/* Thread_pause is called by the currently running thread */
void Thread_pause(struct uthread_queue *uq) {
  assert(uq->curr_thr);
  // assert(n_threads > 1);
  ready_enqueue(uq->curr_thr, uq->ready_q);
  void *from = uq->curr_thr;
  void *to = uq->curr_thr = ready_dequeue(uq->ready_q);
  _swtch(from, to);
}

char join_0_called = 0;

int Thread_join(unsigned long tid, struct uthread_queue *uq) {
  // assert(curr_thr && curr_thr->id != tid && !join_0_called);
  struct u_thread *th_to_join = (struct u_thread *)tid;
  if (tid != 0) {                  // tid is also the address of the thread
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
    // wait for all threads
    if (uq->n_threads > 1) {
      uq->join0_thread = uq->curr_thr;
      Thread_pause(uq);
    }
    return 0;
  }
}

// int mytestfunc(void *args) {
//   int arg = *(int *)args;
//   printf("Thread %lu is running with arg=%d\n", curr_thr->id, arg);
//   for (int i = 0; i < 250000; ++i) {
//     ++arg;
//   }
//   printf("Thread %lu spinned 25k times into ++arg=%d\n", curr_thr->id, arg);
//   return Thread_self();
// }

// int myjointestfunc(void *args) {
//   int arg = *(int *)args;
//   int arg1 = arg + 1000;
//   printf("Thread %lu is running with arg=%d and will create a thread with "
//          "arg=%d and join it\n",
//          curr_thr->id, arg, arg1);

//   Thread_join(Thread_new(mytestfunc, (void *)(&arg1), sizeof(int), NULL));

//   printf("Thread %lu returned after calling join on thread with arg=%d\n",
//          curr_thr->id, arg1);
//   return Thread_self();
// }

// int main() {
//   int arg1 = 4, arg2 = 5, arg3 = 6, arg4 = 7;
//   Thread_init();
//   int thid1 = Thread_new(mytestfunc, (void *)&arg1, sizeof(int), NULL);
//   int thid2 = Thread_new(mytestfunc, (void *)&arg2, sizeof(int), NULL);
//   int thid3 = Thread_new(myjointestfunc, (void *)&arg3, sizeof(int), NULL);
//   int thid4 = Thread_new(mytestfunc, (void *)&arg4, sizeof(int), NULL);
//   Thread_join(0);
//   printf("Returned to main successfully\n");
// }

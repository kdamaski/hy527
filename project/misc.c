#include "uthread.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void push_to_free(struct u_thread *th, struct u_thread *free_list) {
  // assert(free_list);
  th->to_run = 0;
  th->next = free_list;
  free_list = th;
}

void ready_enqueue(struct u_thread *th,
                   struct th_ready_q *ready_q) { // make it circular
  if (ready_q->head == NULL) {
    ready_q->head = ready_q->tail = th;
  } else {
    ready_q->tail->next = th;
    ready_q->tail = th;
  }
  ready_q->tail->next = NULL;
}

// remove from head
struct u_thread *ready_dequeue(struct th_ready_q *ready_q) {
  if (ready_q->head == NULL) {
    fprintf(stderr, "Cannot dequeue from empty\n");
    return NULL;
  }
  struct u_thread *out = ready_q->head;
  ready_q->head = ready_q->head->next;

  return out;
}

void print_Q(struct uthread_queue *uq) {
  assert(uq->curr_thr);
  printf("Current thread running is %p in queue with main thread %p\n",
         uq->curr_thr, uq->main_thr);
  if (!uq->ready_q->head) {
    printf("Q empty\n");
    return;
  } else {
    struct u_thread *tmp = uq->ready_q->head;
    while (tmp) {
      printf("Thread %p is in ready_q of main thread %p\n", tmp, uq->main_thr);
      tmp = tmp->next;
    }
  }
}

/* frees the stack memory of threads that have exited */
void empty_free(struct u_thread *free_list) {
  // assert(free_list);
  struct u_thread *tmp = free_list;
  while (tmp) {
    free_list = free_list->next;
    free(tmp);
    tmp = free_list;
  }
}

void push_to_join(struct u_thread *th, struct u_thread *curr_thr) {
  if (th->jlist)
    curr_thr->next = th->jlist->next;
  else
    curr_thr->next = NULL;
  th->jlist = curr_thr;
}

// never used
struct u_thread *pop_from_join(struct u_thread *join0_thread) {
  // assert(join0_thread);
  struct u_thread *tmp = join0_thread;
  if (tmp)
    join0_thread = join0_thread->next;
  return tmp;
}

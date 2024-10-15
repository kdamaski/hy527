#ifndef THREAD_H
#define THREAD_H
#include "thread.h"
#include <stdio.h>

extern struct thread_queue *thr_q;
extern unsigned n_threads;

void thr_enqueue(struct damthread *th) {
  if (thr_q->head == NULL) {
    thr_q->head = thr_q->tail = th;
    thr_q->head->next = thr_q->tail->next = NULL;
  } else {
    thr_q->tail->next = th;
    th->next = NULL;
    thr_q->tail = th;
  }
  ++n_threads;
}

// remove from head
struct damthread *thr_dequeue() {
  if (thr_q->head == NULL) {
    fprintf(stderr, "Cannot dequeue from empty\n");
    return NULL;
  }
  struct damthread *out = thr_q->head;
  thr_q->head = thr_q->head->next;
  return out; // NOTE i am not calling free
  --n_threads;
}

void print_Q() {
  if (!thr_q->head) {
    printf("Q empty\n");
    return;
  } else {
    struct damthread *tmp = thr_q->head;
    while (tmp != NULL) {
      printf("Thread %u with arg: %d\n", tmp->id, *(int *)tmp->args);
      tmp = tmp->next;
    }
  }
}

#endif /* ifndef DEBUG */

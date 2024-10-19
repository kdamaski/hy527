#include "misc.h"
#include "thread.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern struct thread_queue *thr_q;
extern struct freelist *free_list;
extern unsigned n_threads;

void insert_to_free(struct damthread *th) {
  // assert(free_list);
  th->next = free_list->next;
  free_list->head = th;
}

void thr_enqueue(struct damthread *th) {
  if (thr_q->head == NULL) {
    thr_q->head = thr_q->tail = th;
    thr_q->head->next = thr_q->tail->next = NULL;
  } else {
    thr_q->tail->next = th;
    th->next = NULL;
    thr_q->tail = th;
  }
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
}

void print_Q() {
  if (!thr_q->head) {
    printf("Q empty\n");
    return;
  } else {
    struct damthread *tmp = thr_q->head;
    while (tmp != NULL) {
      printf("Thread %lu with arg: %d\n", tmp->id, *(int *)(tmp->args));
      tmp = tmp->next;
    }
  }
}

/* frees the stack memory of threads that have exited */
void empty_free() {
  assert(free_list);
  struct damthread *tmp = free_list->head;
  while (tmp) {
    free(tmp);
    free_list->head = free_list->next;
    tmp = free_list->head;
  }
}

// returns 0 if not found
struct damthread *thread_exists(unsigned tid) {
  struct damthread *tmp = thr_q->head;
  while (tmp) {
    if (tmp->id == tid)
      return tmp;
    tmp = tmp->next;
  }
  return NULL;
}

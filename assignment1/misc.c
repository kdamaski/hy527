// #include "misc.h" was prompting error conflicting types
#include "thread.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern struct th_ready_q *ready_q;
extern struct u_thread *free_list;
extern struct u_thread *join_list;
extern unsigned n_threads;

void push_to_free(struct u_thread *th) {
  // assert(free_list);
  th->to_run = 0;
  // th->prev->next = th->next;
  // th->next->prev = th->prev;
  th->next = free_list;
  free_list = th;
}

void ready_enqueue(struct u_thread *th) { // make it circular
  if (ready_q->head == NULL) {
    ready_q->head = ready_q->tail = th;
  } else {
    ready_q->tail->next = th;
    // th->prev = ready_q->tail;
    // th->next = ready_q->head;
    th->next = ready_q->tail->next;
    ready_q->tail = th;
  }
  ready_q->tail->next = NULL;
}

// remove from head
struct u_thread *ready_dequeue() {
  if (ready_q->head == NULL) {
    fprintf(stderr, "Cannot dequeue from empty\n");
    return NULL;
  }
  struct u_thread *out = ready_q->head;
  // ready_q->tail->next = out->next; // could be head ofc
  ready_q->head = ready_q->head->next;
  // ready_q->head->prev = ready_q->tail;

  return out; // NOTE i am not calling free
}

void print_Q() {
  if (!ready_q->head) {
    printf("Q empty\n");
    return;
  } else {
    struct u_thread *tmp = ready_q->head->next;
    printf("Thread %lu with arg: %d\n", ready_q->head->id,
           *(int *)(ready_q->head->sp[2]));
    while (tmp != ready_q->head) {
      printf("Thread %lu with arg: %d\n", tmp->id, *(int *)(tmp->sp[2]));
      tmp = tmp->next;
    }
  }
}

/* frees the stack memory of threads that have exited */
void empty_free() {
  // assert(free_list);
  struct u_thread *tmp = free_list;
  while (tmp) {
    free_list = free_list->next;
    free(tmp);
    tmp = free_list;
  }
}

void push_to_join(struct u_thread *th) {
  // head awaits for me
  struct u_thread *newj = ready_dequeue();
  if (th->jlist)
    newj->next = th->jlist->next;
  else
    newj->next = NULL;
  th->jlist = newj;
}

// never used
struct u_thread *pop_from_join() {
  // assert(join_list);
  struct u_thread *tmp = join_list;
  if (tmp)
    join_list = join_list->next;
  return tmp;
}

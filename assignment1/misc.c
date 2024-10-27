// #include "misc.h" was prompting error conflicting types
#include "thread.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern struct th_ready_q *ready_q;
extern struct u_thread *free_list;
extern struct u_thread *join0_thread;
extern struct u_thread *curr_thr;
extern unsigned n_threads;

extern unsigned long *main_esp;
extern unsigned long *main_ebp;

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

  return out;
}

void print_Q() {
  assert(curr_thr);
  printf("Current thread running is %p\n", curr_thr);
  if (!ready_q->head) {
    printf("Q empty\n");
    return;
  } else {
    struct u_thread *tmp = ready_q->head;
    while (tmp) {
      printf("Thread %p is in ready_q\n", tmp);
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
  if (th->jlist)
    curr_thr->next = th->jlist->next;
  else
    curr_thr->next = NULL;
  th->jlist = curr_thr;
}

// never used
struct u_thread *pop_from_join() {
  // assert(join0_thread);
  struct u_thread *tmp = join0_thread;
  if (tmp)
    join0_thread = join0_thread->next;
  return tmp;
}

void restore_context() {
  // curr_thr->sp = main_esp;
  // switch esp to the last thread available
  // asm volatile("movl [main_esp],%%esp\n\t" ::"r"(main_esp));
  // asm volatile("movl [main_ebp],%%ebp\n\t" ::"r"(main_ebp));
  // asm volatile("movl  4(%esp),%ebp\n\t"
  //              "ret\n\t");

  // akyro to apo katw?
  // asm volatile("movl  %0,%%esp\n\t" ::"r"(curr_thr->sp));

  // asm volatile("movl  %0,%%esp\n\t" ::"r"(curr_thr->sp));
  // asm volatile("movl  0(%esp),%ebx\n\t"
  //              "movl  4(%esp),%esi\n\t"
  //              "movl  8(%esp),%edi\n\t"
  //              "movl  12(%esp),%ebp\n\t"
  //              "addl  $16, %esp\n\t"
  //              "ret\n\t");
}

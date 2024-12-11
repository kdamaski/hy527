#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

typedef struct u_thread {
  unsigned long *sp;
  unsigned long id;
  struct u_thread *next;
  struct u_thread *jlist; // a jlist for thread
  char to_run;            // i mainly care for free or active
} u_thread;

struct th_ready_q {
  struct u_thread *head;
  struct u_thread *tail;
};

struct uthread_queue {
  struct th_ready_q *ready_q;
  struct u_thread *join0_thread;
  struct u_thread *curr_thr;
  struct u_thread *main_thr;
  struct u_thread *free_list;
  unsigned stack_size;
  unsigned n_threads;
};

#endif

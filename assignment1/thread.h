#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <stddef.h>

void Thread_init(void);
int Thread_new(int func(void *), void *args, size_t nbytes, ...);
void Thread_exit(int code);
int Thread_self(void);
int Thread_join(unsigned long tid);
void Thread_pause(void);

// enum thr_queue { READYQ = 0, FREEQ };

struct u_thread {
  unsigned long *sp;
  int (*pc)(void *);
  void *args;
  unsigned long id;
  struct u_thread *next;
  struct u_thread *jlist; // a jlist for thread
  char to_run;            // i mainly care for free or artive
};

struct th_ready_q {
  struct u_thread *head;
  struct u_thread *tail;
};

#endif

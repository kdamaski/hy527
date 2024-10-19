#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <stddef.h>

void Thread_init(void);
int Thread_new(int func(void *), void *args, size_t nbytes, ...);
void Thread_exit(int code);
int Thread_self(void);
int Thread_join(unsigned tid);
void Thread_pause(void);

struct damthread {
  unsigned long *sp;
  int (*pc)(void *);
  void *args;
  unsigned long id;
  struct damthread *thr_joining; // threads that wait on me
  // NOTE perhaps add a flag or pointer to show which Q thread is in
  struct damthread *next;
};

struct thread_queue {
  struct damthread *head;
  struct damthread *tail;
};

struct freelist {
  struct damthread *head;
  struct damthread *next;
};

#endif

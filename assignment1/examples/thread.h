
#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <stddef.h>

void Thread_init(void);
int Thread_new(int func(void *), void *args, size_t nbytes, unsigned id);
void Thread_exit(int code);
int Thread_self(void);
int Thread_join(int tid);
void Thread_pause(void);

struct damthread {
  long int rsp;
  long int rbp;
  long int rip;
  int (*func)(void *);
  void *args;
  size_t nbytes; // size of arguments
  unsigned id;
  struct damthread *next;
};

struct thread_queue {
  struct damthread *next;
  struct damthread *head;
  struct damthread *tail;
};

#endif

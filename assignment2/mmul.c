









































#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>

#define MAX_THREADS 8

typedef struct{
 int needed;
 int called;
 pthread_mutex_t mutex;
 pthread_cond_t cond;
} barrier_t;

void barrier_init(barrier_t *barrier)
{
    barrier->needed = -1;
    barrier->called = 0;
    pthread_mutex_init(&barrier->mutex,NULL);
    pthread_cond_init(&barrier->cond,NULL);
}

void barrier_wait(barrier_t *barrier, int cpu_no)
{
    pthread_mutex_lock(&barrier->mutex);
    if (barrier->needed == -1) barrier->needed = cpu_no;
    if (barrier->needed != cpu_no) {printf("Error in appl\n");}
    barrier->called++;
    if (barrier->called == barrier->needed) {
        barrier->called = 0;
        barrier->needed = -1;
        pthread_cond_broadcast(&barrier->cond);
    } else {
        pthread_cond_wait(&barrier->cond,&barrier->mutex);
    }
    pthread_mutex_unlock(&barrier->mutex);
}

struct timeval the_time;
pthread_cond_t barrier_cond;
pthread_mutex_t barrier_mutex;
int thread_count = 0;
pthread_t threads[MAX_THREADS];
int id = 0;
pthread_mutex_t id_lock;;
 /* standard libraries structs, functions, macros (e.g MAX_THREADS 8) */
#include "init.h"
#define N 100
#define DEFAULT_P 1

double A[N][N], B[N][N];

int P = DEFAULT_P;

int thread_count = 0;
unsigned int start;

int main(int argc, char *argv[]) {
  init_matrices("matrixfile", A, B, N);
  
  
pthread_mutex_init((pthread_mutex_t*)&(id_lock), NULL);;
pthread_mutex_init(&barrier_mutex, NULL);
pthread_cond_init(&barrier_cond, NULL);
pthread_mutex_lock(&barrier_mutex);
pthread_mutex_unlock(&barrier_mutex);
; // inits locks and barriers

  while ((c = getopt(argc, argv, "p")) != -1) {
    switch(c) {
      case 'p': P = atoi(optarg); 
        if (P < 1 || P > 8) {
          fprintf(stderr, "P must be >= 1 and <= 8\n");
          exit(-1);
        }
        break;  
      default:
        break;
    }
  for (i=1; i<P; i++) {
    
pthread_create(&threads[thread_count],NULL,SlaveStart,NULL);
thread_count++;
; // calls pthread_create and does ++thread_count
  }
  {
                  unsigned long now;
                  gettimeofday(&the_time, NULL);
                  now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
                  start = (unsigned int)now;
               };

  return 0;
}

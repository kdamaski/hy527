#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 8

typedef struct {
  int needed;
  int called;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} barrier_t;

void barrier_init(barrier_t *barrier) {
  barrier->needed = -1;
  barrier->called = 0;
  pthread_mutex_init(&barrier->mutex, NULL);
  pthread_cond_init(&barrier->cond, NULL);
}

void barrier_wait(barrier_t *barrier, int cpu_no) {
  pthread_mutex_lock(&barrier->mutex);
  if (barrier->needed == -1)
    barrier->needed = cpu_no;
  if (barrier->needed != cpu_no) {
    printf("Error in appl\n");
  }
  barrier->called++;
  if (barrier->called == barrier->needed) {
    barrier->called = 0;
    barrier->needed = -1;
    pthread_cond_broadcast(&barrier->cond);
  } else {
    pthread_cond_wait(&barrier->cond, &barrier->mutex);
  }
  pthread_mutex_unlock(&barrier->mutex);
}

struct timeval the_time;
pthread_cond_t barrier_cond;
pthread_mutex_t barrier_mutex;
int thread_count = 0;
pthread_t threads[MAX_THREADS];
int id = 0;
pthread_mutex_t id_lock;
;
/* standard libraries structs and functions */
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096 /* rudro - I have this already */
#endif

#define NUM_CACHE_LINES 65536
#define LOG2_LINE_SIZE 12
#define DEFAULT_M 18
#define DEFAULT_P 1

struct GlobalMemory {
  int id;
  pthread_mutex_t idlock;
  barrier_t start;
  int *transtimes;
  int *totaltimes;
  clock_t starttime;
  clock_t finishtime;
  int initdonetime;
} *Global;

int P = DEFAULT_P;
int M = DEFAULT_M;
int N; /* N = 2^M                                */

int rootN;      /* rootN = N^1/2                          */
double *x;      /* x is the original time-domain data     */
double *trans;  /* trans is used as scratch space         */
double *umain;  /* umain is roots of unity for 1D FFTs    */
double *umain2; /* umain2 is entire roots of unity matrix */

int orig_num_lines = NUM_CACHE_LINES;  /* number of cache lines */
int num_cache_lines = NUM_CACHE_LINES; /* number of cache lines */
int log2_line_size = LOG2_LINE_SIZE;
int line_size;
int rowsperproc;
int pad_length;

int main(int argc, char *argv[]) {
  int i;
  int j;
  int c;
  extern char *optarg;
  int m1;
  int factor;
  int pages;
  unsigned int start;

  int starttime, finishtime; /* rudro - some variables in yzhou's lib */

  pthread_mutex_init((pthread_mutex_t *)&(id_lock), NULL);
  ;
  pthread_mutex_init(&barrier_mutex, NULL);
  pthread_cond_init(&barrier_cond, NULL);
  pthread_mutex_lock(&barrier_mutex);
  pthread_mutex_unlock(&barrier_mutex);
  ;

  {
    unsigned long now;
    gettimeofday(&the_time, NULL);
    now = ((the_time.tv_sec - 879191283) * 1000) + (the_time.tv_usec / 1000);
    start = (unsigned int)now;
  };

  while ((c = getopt(argc, argv, "i:p:m:n:l:stoh")) != -1) {
    switch (c) {
    case 'p':
      P = atoi(optarg);
      if (P < 1) {
        fprintf(stderr, "P must be >= 1\n");
        exit(-1);
      }
      // if (log_2(P) == -1) {
      if (P % 2 != 0) {
        // fprintf(stderr, "P must be a power of 2\n");
        fprintf(stderr, "P must be even\n");
        exit(-1);
      }
      break;
    case 'm':
      M = atoi(optarg);
      m1 = M / 2;
      if (2 * m1 != M) {
        fprintf(stderr, "M must be even\n");
        exit(-1);
      }
      break;
    case 'n':
      num_cache_lines = atoi(optarg);
      orig_num_lines = num_cache_lines;
      if (num_cache_lines < 1) {
        fprintf(stderr, "Number of cache lines must be >= 1\n");
        exit(-1);
      }
      break;
    case 'l':
      log2_line_size = atoi(optarg);
      if (log2_line_size < 0) {
        fprintf(stderr,
                "Log base 2 of cache line length in bytes must be >= 0\n");
        exit(-1);
      }
      break;
    case 'h':
      printf("Usage: kostas-parallel <options>\n\n");
      printf("options:\n");
      printf("  -mM : M = even integer; 2**M total problem size\n");
      printf("  -pP : P = number of processors; Must be a power of 2.\n");
      printf("  -nN : N = number of cache lines.\n");
      printf("  -lL : L = Log base 2 of cache line length in bytes.\n");
      printf("  -h  : Print out command line options.\n\n");
      printf("Default: kostas-parallel -m%1d -p%1d -n%1d -l%1d\n", DEFAULT_M,
             DEFAULT_P, NUM_CACHE_LINES, LOG2_LINE_SIZE);
      exit(0);
      break;
    }
  }

  N = 1 << M;
  rootN = 1 << (M / 2);
  rowsperproc = rootN / P;
  if (rowsperproc == 0) {
    fprintf(stderr, "Matrix not large enough. 2**(M/2) must be >= P\n");
    exit(-1);
  }

  line_size = 1 << log2_line_size;
  if (line_size < 2 * sizeof(double)) {
    printf("WARNING: Each element is a complex double (%d bytes)\n",
           2 * sizeof(double));
    printf("  => Less than one element per cache line\n");
    printf("     Computing transpose blocking factor\n");
    factor = (2 * sizeof(double)) / line_size;
    num_cache_lines = orig_num_lines / factor;
  }
  if (line_size <= 2 * sizeof(double)) {
    pad_length = 1;
  } else {
    pad_length = line_size / (2 * sizeof(double));
  }

  if (rowsperproc * rootN * 2 * sizeof(double) >= PAGE_SIZE) {
    pages = (2 * pad_length * sizeof(double) * rowsperproc) / PAGE_SIZE;
    if (pages * PAGE_SIZE != 2 * pad_length * sizeof(double) * rowsperproc) {
      pages++;
    }
    pad_length = (pages * PAGE_SIZE) / (2 * sizeof(double) * rowsperproc);
  } else {
    pad_length = (PAGE_SIZE - (rowsperproc * rootN * 2 * sizeof(double))) /

                 (2 * sizeof(double) * rowsperproc);
    if (pad_length * (2 * sizeof(double) * rowsperproc) !=
        (PAGE_SIZE - (rowsperproc * rootN * 2 * sizeof(double)))) {
      fprintf(stderr, "Padding algorithm unsuccessful\n");
      exit(-1);
    }
  }

  /* rudro */ pad_length = 0;

  printf("alloc: Global: %d pages\n", sizeof(struct GlobalMemory) / PAGE_SIZE);
  Global = (struct GlobalMemory *)malloc(sizeof(struct GlobalMemory));
  ;

  printf("alloc: x: %d pages\n",
         (2 * (N + rootN * pad_length) * sizeof(double) + PAGE_SIZE) /
             PAGE_SIZE);
  x = (double *)malloc(2 * (N + rootN * pad_length) * sizeof(double) +
                       PAGE_SIZE);
  ;

  printf("alloc: trans: %d pages\n",
         (2 * (N + rootN * pad_length) * sizeof(double) + PAGE_SIZE) /
             PAGE_SIZE);
  trans = (double *)malloc(2 * (N + rootN * pad_length) * sizeof(double) +
                           PAGE_SIZE);
  ;

  printf("alloc: umain: %d pages\n", (2 * rootN * sizeof(double)) / PAGE_SIZE);
  umain = (double *)malloc(2 * rootN * sizeof(double));
  ;

  printf("alloc: umain2: %d pages\n",
         (2 * (N + rootN * pad_length) * sizeof(double) + PAGE_SIZE) /
             PAGE_SIZE);
  umain2 = (double *)malloc(2 * (N + rootN * pad_length) * sizeof(double) +
                            PAGE_SIZE);
  ;

  Global->transtimes = (int *)malloc(P * sizeof(int));
  ;
  Global->totaltimes = (int *)malloc(P * sizeof(int));
  ;
  if (Global == NULL) {
    fprintf(stderr, "Could not malloc memory for Global\n");
    exit(-1);
  } else if (x == NULL) {
    fprintf(stderr, "Could not malloc memory for x\n");
    exit(-1);
  } else if (trans == NULL) {
    fprintf(stderr, "Could not malloc memory for trans\n");
    exit(-1);
  } else if (umain == NULL) {
    fprintf(stderr, "Could not malloc memory for umain\n");
    exit(-1);
  } else if (umain2 == NULL) {
    fprintf(stderr, "Could not malloc memory for umain2\n");
    exit(-1);
  }

  x = (double *)(((uint64_t)x) + PAGE_SIZE - ((uint64_t)x) % PAGE_SIZE);
  trans =
      (double *)(((uint64_t)trans) + PAGE_SIZE - ((uint64_t)trans) % PAGE_SIZE);
  umain2 = (double *)(((uint64_t)umain2) + PAGE_SIZE -
                      ((uint64_t)umain2) % PAGE_SIZE);

  /* In order to optimize data distribution, the data structures x, trans,
     and umain2 have been aligned so that each begins on a page boundary.
     This ensures that the amount of padding calculated by the program is
     such that each processor's partition ends on a page boundary, thus
     ensuring that all data from these structures that are needed by a
     processor can be allocated to its local memory */

  /* POSSIBLE ENHANCEMENT:  Here is where one might distribute the x,
     trans, and umain2 data structures across physically distributed
     memories as desired.

     One way to place data is as follows:

     double *base;
     int i;

     i = ((N/P)+(rootN/P)*pad_length)*2;
     base = &(x[0]);
     for (j=0;j<P;j++) {
      Place all addresses x such that (base <= x < base+i) on node j
      base += i;
     }

     The trans and umain2 data structures can be placed in a similar manner.

     */

  return 0;
}

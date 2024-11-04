#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define f(x) ((float)(4.0/(1.0+x*x)))
#define pi ((float)(4.0*atan(1.0)))

main()

{

/* This simple program approximates pi by computing pi = integral
 * from 0 to 1 of 4/(1+x*x)dx which is approximated by sum 
 * from k=1 to N of 4 / ((1 + (k-1/2)**2 ).  The only input data 
 * required is N.                                       
*/

/* Each process is given a chunk of the interval to do. */

float err, sum, w;
int i, N;
void startup();

/* startup(&mynum, &nprocs) */

/* 
 * Now solicit a new value for N.  When it is 0, then you should depart.
 * This would be a good place to unenroll yourself as well.
*/

printf ("Enter number of approximation intervals:(0 to exit)\n");
scanf("%d",&N);

while (N > 0) 
  {
   w = 1.0/(float)N;
   sum = 0.0;
   for (i = 1; i <= N; i++)
      sum = sum + f(((float)i-0.5)*w);
   sum = sum * w;
   err = sum - pi;
   printf("sum, err = %7.5f, %10e\n", sum, err);
   printf ("Enter number of approximation intervals:(0 to exit)\n");
   scanf("%d",&N);
   }
}

void startup (pmynum, pnprocs)
int *pmynum, *pnprocs;
{
/* logic executed at the start of the program (which is identical for
 * for the host and node programs).  This is the SPMD model of programming.
 *
 * This is a good place to enroll yourself, returning "mynum"
*/
}

/* int_pi1.c
 * This simple program approximates pi by computing pi = integral
 * from 0 to 1 of 4/(1+x*x)dx which is approximated by sum 
 * from k=1 to N of 4 / ((1 + (k-1/2)**2 ).  The only input data
 * required is N.
 * Parallel version number 1:                 (int_pi1.c) 	API
 * All instances are started at load time but the first does all the
 * work.
 * Revised: 4/9/93 bbarney
 * Revised: 5/28/93 riordan
 * Revised: 10/11/94 zollweg
 * Converted to MPI: 11/12/94 Xianneng Shen
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"
#define f(x) ((float)(4.0/(1.0+x*x)))
#define pi ((float)(4.0*atan(1.0)))
MPI_Status status;
main(int argc, char **argv) 
{

float 	err, 
	sum, 
	w;
int 	i, 
	N, 
	mynum, 
	nprocs;

/* All instances call startup routine to get their instance number (mynum) 
 * We'll also return nprocs, which the main program will need in the 
 * next revision.
*/
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &mynum);
MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
printf("nprocs = %d, mynum=%d\n", nprocs, mynum);
/* Step (1): get a value for N, the number of intervals in the approximation
 * (Parallel versions: the initial instance, or master, reads this in.)
 * This would be a good place to add message-passing, so that the master
 * can send N to the other nodes, and the other nodes can get N from the
 * master.  
*/
if (mynum == 0) {
   printf ("Enter number of approximation intervals:(0 to exit)\n");
   scanf("%d",&N);
   }
else N=0;

/* Step (2): check for exit condition. */
if (N <= 0) {
   printf("node %d left\n", mynum);
/* pvm_exit(); */
   exit(0);
   }

/* Step (3): do the computation in N steps
 * (Ultimately, this work should be divided up among the processes)
 */
while (N > 0) {
   w = 1.0/(float)N;
   sum = 0.0;
   for (i = 1; i <= N; i++)
      sum = sum + f(((float)i-0.5)*w);
   sum = sum * w;
   err = sum - pi;

/* Step (4): print the results  
 * (Ultimately, partial results will have to be sent to the master,
 *  who will then print the answer)
 */
   printf("sum, err = %7.5f, %10e\n", sum, err);
   if (mynum == 0) {
      printf ("Enter number of approximation intervals:(0 to exit)\n");
      scanf("%d",&N);
      }
   }
  exit(0);
  MPI_Finalize();
}

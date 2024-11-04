
OMPI_MPICC = gcc
export OMPI_MPICC

CC 	=	mpicc
LIBS    =       

default: mmul_distributed

mmul_distributed:	mul_mpi.c
	${CC} mul_mpi.c ${LIBS} -o mmul_distributed

clean:
	rm mmul_distributed

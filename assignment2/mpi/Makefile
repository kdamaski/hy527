# make.int_pi1.c
OMPI_MPICC = gcc
export OMPI_MPICC

CC 	=	mpicc
LIBS    =       

all:	int_pi1 int_pi2

int_pi1:	int_pi1.c
	${CC} int_pi1.c ${LIBS} -o int_pi1

int_pi2:	int_pi2.c
	${CC} int_pi2.c ${LIBS} -o int_pi2

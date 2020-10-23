PROG=jacobi

CC ?= gcc
FLAGS_OPENMP ?= -fopenmp

debug:
	$(CC) -Wall -g $(FLAGS_OPENMP) -O0 ${PROG}.c -o ${PROG}.o -lm

release:
	$(CC) -Wall -g $(FLAGS_OPENMP) -O3 ${PROG}.c -o ${PROG}.o -lm

run go:
	OMP_DYNAMIC=false ./${PROG}.o < input

clean:
	rm -f ${PROG}.o ${PROG}.o *~

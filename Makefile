CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall

all: odd_even_serial odd_even_openmp odd_even_mpi

odd_even_serial: odd_even_serial.c
	$(CC) $(CFLAGS) -o odd_even_serial odd_even_serial.c

odd_even_openmp: odd_even_openmp.c
	$(CC) $(CFLAGS) -fopenmp -o odd_even_openmp odd_even_openmp.c

#odd_even_mpi: odd_even_mpi.c
#	$(MPICC) $(CFLAGS) -o odd_even_mpi odd_even_mpi.c

clean:
	rm -f odd_even_serial odd_even_openmp odd_even_mpi

test: all
	./odd_even_serial 1000
	./odd_even_openmp 1000 2

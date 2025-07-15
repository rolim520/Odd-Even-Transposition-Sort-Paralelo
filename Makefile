CC = gcc
CFLAGS = -O2 -Wall -D_POSIX_C_SOURCE=199309L
LDFLAGS_OPENMP = -fopenmp
LDFLAGS_MPI = 

TARGET_SERIAL = build/odd_even_serial
TARGET_OPENMP = build/odd_even_openmp
TARGET_MPI = build/odd_even_mpi

all: $(TARGET_SERIAL) $(TARGET_OPENMP) $(TARGET_MPI)

$(TARGET_SERIAL): odd_even_serial.c
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_OPENMP): odd_even_openmp.c
	$(CC) $(CFLAGS) $(LDFLAGS_OPENMP) -o $@ $^

$(TARGET_MPI): odd_even_mpi.c
	$(CC) $(CFLAGS) $(LDFLAGS_MPI) -o $@ $^

clean:
	rm -rf build
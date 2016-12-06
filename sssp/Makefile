CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpic++

CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x
LDFLAGS			:= -lX11 -fopenmp -lm
all: SSSP_MPI_sync SSSP_MPI_async
SSSP_MPI_sync: SSSP_common.o SSSP_MPI_sync.o
	mpicc $(CCFLAGS) $(LDFLAGS) -o $@ $?
SSSP_MPI_async: SSSP_common.o SSSP_MPI_async.o
	mpicc $(CCFLAGS) $(LDFLAGS) -o $@ $?
SSSP_common.o: SSSP_common.c include.h
	mpicc $(CCFLAGS) $(LDFLAGS) -c SSSP_common.c -o $@
SSSP_MPI_sync.o: SSSP_MPI_sync.c include.h
	mpicc $(CCFLAGS) $(LDFLAGS) -c SSSP_MPI_sync.c -o $@
SSSP_MPI_async.o: SSSP_MPI_async.c include.h
	mpicc $(CCFLAGS) $(LDFLAGS) -c SSSP_MPI_async.c -o $@
clean: rm SSSP_MPI_async SSSP_MPI_async SSSP_common.o SSSP_MPI_sync.o SSSP_MPI_async.o

CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpic++

CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x
LDFLAGS			:= -lX11 -fopenmp -lm
all: SSSP_MPI_sync SSSP_MPI_async SSSP_Pthread
SSSP_MPI_sync: SSSP_common.o SSSP_MPI_sync.c tool.o
	mpicc $(CCFLAGS) $(LDFLAGS) -o $@ $?
SSSP_MPI_async: SSSP_common.o SSSP_MPI_async.c tool.o
	mpicc $(CCFLAGS) $(LDFLAGS) -o $@ $?
SSSP_Pthread: SSSP_common.o SSSP_Pthread.c tool.o
	mpicc -lpthread $(CCFLAGS) $(LDFLAGS) -o $@ $?
tool.o: tool.c include.h
	mpicc $(CCFLAGS) $(LDFLAGS) -c tool.c -o $@
SSSP_common.o: SSSP_common.c include.h
	mpicc $(CCFLAGS) $(LDFLAGS) -c SSSP_common.c -o $@

clean:
	rm SSSP_Pthread SSSP_MPI_async SSSP_MPI_sync SSSP_common.o tool.o

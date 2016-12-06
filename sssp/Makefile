CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpic++

CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x
LDFLAGS			:= -lX11 -fopenmp -lm

mpi: mpi_common.o sssp_common.o
	mpicc $(CCFLAGS) $(LDFLAGS) -o $@ $?

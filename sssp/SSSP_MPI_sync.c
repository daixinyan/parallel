#include "include.h"

void my_mpi_execute();


int main(int argc,char *argv[])
{
    my_mpi_init(argc, argv);
    my_mpi_execute();
    my_mpi_finalize();
    return 0;
}


void my_mpi_execute()
{
    int hasMessage = rank == source_vertex;
    int loop;
    do
    {
        
        myAllreduce(&hasMessage, &loop, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
    }while(loop);
}

#include "include.h"


int main(int argc,char *argv[])
{

    /**record time**/
    double total_start_time;
    double total_end_time;

    /**init mpi**/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    total_start_time = MPI_Wtime();


    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

		if(PRINT_TIME)
    {
        printf( "rank: %d\n total_time: %f\n communication_time: %f\n compution_time: %f\n",
                 rank, total_time, communication_time, compution_time );
    }

    MPI_Finalize();
}

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define PRINT_TIME 1
#define IF_PRINT   1

/**start struct meta**/
    int size,rank;
/**end struct**/


/**start struct time**/
double communication_time = .0;
double compution_time = .0;
double total_time = .0;
/**end struct time**/



/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
int myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
int mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );



int main(int argc,char *argv[])
{

    /**record time**/
    double total_start_time;
    double total_end_time;

    double start_time;
    double end_time;


    /**init mpi**/
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

    total_start_time = MPI_Wtime();


    /**init excute parameters.**/
        if(argc<3)
        {
        }
        else
        {
        }

    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

    if(PRINT_TIME)
    {
        printf("total_time: %f\n communication_time: %f\n compution_time: %f\n read_time: %f\n write_time: %f\n",
            total_time, communication_time, compution_time, read_time, write_time );
    }

    MPI_Finalize();
}




void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status)
{
  double start_time;
  double end_time;
  start_time = MPI_Wtime();
  MPI_Sendrecv(sendbuf, sendcount, sendtype,dest, sendtag,
    recvbuf, recvcount, recvtype,source, recvtag, comm,  status);
  end_time = MPI_Wtime();
  communication_time += end_time-start_time;
}



int myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}




int mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

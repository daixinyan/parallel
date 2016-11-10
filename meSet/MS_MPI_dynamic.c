#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <X11/Xlib.h>

#define PRINT_TIME 1
#define IF_PRINT   1



typedef struct complextype
{
	double real, imag;
} Compl;

typedef struct Task{
  int process_handle_start_x;
  int process_handle_count_x;
}Task;

typedef struct DrawPoint
{
		int repeats;
    short x,y;
}DrawPoint;


struct
{
  int number_of_threads;
  double left_range_of_real;
  double right_range_of_real;
  double real_range;
  double lower_range_of_imag;
  double upper_range_of_imag;
  double imag_range;
  int number_of_points_x;
  int number_of_points_y;
  int is_enable;
}parameters;


/* set window size */
int width = 800;
int height = 800;
/* set window position */

/**start struct meta**/
    int size, rank, actual_size;
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

/**/
void my_init(int argc,char *argv[]);

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



        my_init();



    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

    if(PRINT_TIME)
    {
        printf("total_time: %f\n communication_time: %f\n compution_time: %f\n",
            total_time, communication_time, compution_time );
    }

    MPI_Finalize();
}


void my_init(int argc,char *argv[])
{
  /**init excute parameters.**/
      if(argc<3)
      {
        parameters.number_of_threads = 8;
        parameters.left_range_of_real = -2;
        parameters.right_range_of_real = 2;
        parameters.lower_range_of_imag = -2;
        parameters.upper_range_of_imag = 2;
        parameters.number_of_points_x = 400;
        parameters.number_of_points_y = 400;
        parameters.is_enable = 1;
      }
      else
      {
        parameters.number_of_threads = atoi(argv[1]);
        parameters.left_range_of_real = atof(argv[2]);
        parameters.right_range_of_real = atof(argv[3]);
        parameters.lower_range_of_imag = atof(argv[4]);
        parameters.upper_range_of_imag = atof(argv[5]);
        parameters.number_of_points_x = atoi(argv[6]);
        parameters.number_of_points_y = atoi(argv[7]);
        parameters.is_enable = strlen("enable")==strlen(argv[8]);
      }
      parameters.real_range = parameters.right_range_of_real - parameters.left_range_of_real;
      parameters.imag_range = parameters.upper_range_of_imag - parameters.lower_range_of_imag;
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

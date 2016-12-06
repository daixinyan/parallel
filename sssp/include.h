
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <limits.h>

#define PRINT_TIME 1
#define IF_PRINT   0
#define CENTER_NODE 0


extern int   vertexes_number;
extern int   edges_half_number;

extern int   threads_number;
extern char* input_file_name;
extern char* output_file_name;
extern int   source_vertex;

extern int** graph_weight;

/**record time**/
extern double total_start_time;
extern double total_end_time;
/**start struct time**/
extern double communication_time;
extern double compution_time;
extern double total_time0;
/**end struct time**/

extern int size, rank, actual_size;

/**function for init variables, init MPI.**/
void my_init(int argc,char *argv[]);
void my_finalize();

/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv( const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include "tool.h"

#define PRINT_TIME 0
#define IF_PRINT   0
#define DEBUG      (0)

#define RESULT_TAG 2
#define RESULT_SIZE 1

extern int   vertexes_number;
extern int   edges_half_number;

extern int   threads_number;
extern char* input_file_name;
extern char* output_file_name;
extern int   source_vertex;

extern int** graph_weight;
extern int*  outgoing_vertexes;
extern int outgoing_number;
extern int*  introverted_vertexes;
extern int introverted_number;

/**record time**/
extern double total_start_time;
extern double total_end_time;
/**start struct time**/
extern double communication_time;
extern double compution_time;
extern double barrier_time;
extern double total_time;
extern double fileio_time;
/**end struct time**/

extern MPI_Request *send_request;
extern MPI_Request *recv_request;
extern MPI_Status  *send_status;
extern MPI_Status  *recv_status;

extern int *result_collect;
extern int last_index;

extern int size, rank, actual_size;

/**function for init variables(@see my_mpi_init, init MPI.**/
void my_mpi_init(int argc,char *argv[]);

/**function for init variables**/
void my_init(int argc,char *argv[]);

/**free gloabl variables**/
void my_global_free();
/**function for finalize mpi**/
void my_mpi_finalize();

/**array result**/
void print_result(int *result);

/**when claculate_and_update, done, collect data to one CENTER_NODE**/
void my_collect_and_send();

/**return the rank of next node ,regard the threads as a circle**/
int getNextNodeRank();
/**return 1 if node (@rank point to) is after another node (@vertex_rank point to) else return 0**/
int isAfterVertex(int rank,int vertex_rank);
/**@see MPI_Allreduce and comsumption time to barrier_time**/
void myAllreduce(const void* sendbuf, void* recv_data, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv( const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Isend, add comsumption time to (double)communication_time**/
void myIsend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);

/**@see MPI_Irecv, add comsumption time to (double)communication_time**/
void myIrecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);


#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <string.h>
#include <limits.h>

#define PRINT_TIME 1
#define IF_PRINT   0
#define CENTER_NODE 0

/**function for init variables, init MPI.**/
void  my_init(int argc,char *argv[]);

/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv( const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
  void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status);

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status );

void read_graph();

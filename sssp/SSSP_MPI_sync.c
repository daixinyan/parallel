#include "include.h"
#define LAST_VERTEX 0
#define CURRENT_LENGTH 1

void my_mpi_execute();


int main(int argc,char *argv[])
{
    my_mpi_init(argc, argv);
    my_mpi_execute();
    my_mpi_finalize();
    return 0;
}

void wait_end()
{
  int hasMessage = 0;
  int loop;
  do{
      myAllreduce(&hasMessage, &loop, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
  }while (loop);
}

void my_mpi_execute()
{

    int i;
    int loop;
    int hasMessage;
    int message[2];
    message[LAST_VERTEX] = source_vertex;
    message[]
    if(rank==source_vertex)
    {
        for (i = 0; i < vertexes_number; i++)
        {

        }
        wait_end();
        my_collect();
    }
    else
    {
        hasMessage = graph_weight[source_vertex][rank]!=INT_MAX;
        int loop = 1;
        while (loop)
        {
            sendIntrovertedMessages();
            receiveOutgoingMessage();
            sendOutgoingMessage();
            receveiIntrovertedMessage();
            hasMessage = update_and_calculate();
            myAllreduce(&hasMessage, &loop, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
        }
        sendResult();
    }
}

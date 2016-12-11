#include "include.h"

#define MESSAGE_TYPE_WHITE 1
#define MESSAGE_TYPE_BLACK -1
#define MESSAGE_TYPE_UPDATE 0
#define MESSAGE_TYPE_TERMINATE 31

#define MESSAGE_TYPE 0
#define MESSAGE_LENGTH 1
#define MESSAGE_SIZE 2
int message[MESSAGE_SIZE];
int received[MESSAGE_SIZE];

void malloc_data();
void free_data();
void my_mpi_execute();

int main(int argc,char *argv[])
{
    my_mpi_init(argc, argv);
    my_mpi_execute();
    my_mpi_finalize();
    return 0;
}
void myReceiveMessageAnysource()
{}

void my_mpi_execute()
{
  malloc_data();
  message[MESSAGE_TYPE] = 0;
  message[MESSAGE_LENGTH] = graph_weight[source_vertex][rank];
  last_index = source_vertex;
  if(rank!=source_vertex)
  {
    do {
      /* code */
    } while(1);
  }
  else
  {
    message[MESSAGE_TYPE] = MESSAGE_TYPE_WHITE;
    do {
      mySend(message, getNextNodeRank();
      myReceive(received,actual_size-1);
      if(received[MESSAGE_TYPE]==MESSAGE_TYPE_WHITE)
      {
        //end!
      }else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_BLACK)
      {
        //continue
        message[MESSAGE_TYPE] = MESSAGE_TYPE_WHITE;
      }
    } while(1);
  }
  free_data();
}

void malloc_data()
{}

void free_data()
{}

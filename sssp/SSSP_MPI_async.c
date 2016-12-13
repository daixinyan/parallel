#include "include.h"

#define MESSAGE_TYPE_WHITE 1
#define MESSAGE_TYPE_BLACK -1

#define MESSAGE_TYPE_UPDATE 0
#define MESSAGE_TYPE_TERMINATE 31

#define MESSAGE_TYPE 0
#define MESSAGE_LENGTH 1
#define MESSAGE_SIZE 2
#define MESSAGE_TAG 1

#define RING_INITIAL 0
#define BLACK_ARRIVAL 1
#define WHITE_ARRIVAL 2
#define BLACK_SENT 3
#define WHITE_SENT 4

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



void my_mpi_execute()
{
  malloc_data();
  message[MESSAGE_TYPE] = 0;
  message[MESSAGE_LENGTH] = graph_weight[source_vertex][rank];
  last_index = graph_weight[source_vertex][rank]==INT_MAX?-1:source_vertex;
  if(rank!=source_vertex)
  {
    int ring_state = RING_INITIAL;
    do {
      myReceiveMessageAnysource();
      if(received[MESSAGE_TYPE]==MESSAGE_TYPE_TERMINATE)
      {
        if(ring_state!=WHITE_SENT)
        {
          printf("rank:%d error to end, local work is not done but centtroller node send terminate information!\n",rank );
        }
        break;
      }
      else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_UPDATE)
      {
        if(ring_state==WHITE_SENT)
        {
          ring_state = RING_INITIAL;
        }
        int temp_new_length = recv_data[i][MESSAGE_LENGTH]+graph_weight[introverted_vertexes[i]][rank];
        if( temp_new_length < message[MESSAGE_LENGTH])
        {
          message[MESSAGE_LENGTH] = temp_new_length;
          message[MESSAGE_TYPE] = MESSAGE_TYPE_UPDATE;
          last_index = sender_rank;
          send_message();
        }
      }
      else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_BLACK)
      {
        ring_state = BLACK_ARRIVAL;
      }
      else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_WHITE)
      {
        ring_state = WHITE_ARRIVAL;
      }
    } while(1);
  }
  else
  {
    message[MESSAGE_TYPE] = MESSAGE_TYPE_WHITE;
    do {
      mySend(message, getNextNodeRank());
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

#include "include.h"
#define MESSAGE_SIZE 2
#define STATE 0
#define LENGTH 1
#define MESSAGE_TAG 1
#define MESSAGE_LAST_INDEX 2
#define RESULT_TAG 2
#define RESULT_SIZE 1

int message[MESSAGE_SIZE+1];
int **recv_data;
int *recv_data_temp;

int *result_collect;


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

void wait_end()
{
  int hasMessage = 0;
  int loop;
  do{
      myAllreduce(&hasMessage, &loop, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
  }while (loop);
}


void notify_and_recv()
{
  int i;
  for(i=0; i<outgoing_number; i++)
  {
    if(IF_PRINT)
    {
      printf("rank: %d try to send to %d\n", rank, outgoing_vertexes[i]);
    }
    MPI_Isend(message, MESSAGE_SIZE, MPI_INT, outgoing_vertexes[i], MESSAGE_TAG, MPI_COMM_WORLD, &send_request[i]);
  }
  for(i=0; i<introverted_number; i++)
  {
    if(IF_PRINT)
    {
      printf("rank: %d try to receive from %d\n", rank, outgoing_vertexes[i]);
    }
    MPI_Irecv(recv_data[i], MESSAGE_SIZE, MPI_INT, introverted_vertexes[i], MESSAGE_TAG, MPI_COMM_WORLD, &recv_request[i]);
  }
  MPI_Waitall(outgoing_number, send_request, send_status);
  MPI_Waitall(introverted_number, recv_request, recv_status);
  printf("rank: %d one iteration, done\n", rank);
}


void claculate_and_update()
{
  int i;
  int temp_new_length;

  message[STATE] = 0;
  for(i = 0; i<introverted_number; i++)
  {
      if(recv_data[i][STATE])
      {
        temp_new_length = recv_data[i][LENGTH]+graph_weight[introverted_vertexes[i]][rank];
        if( temp_new_length < message[LENGTH])
        {
          printf("%d from rank %d new length: %d old length: %d\n", rank, introverted_vertexes[i], temp_new_length, message[LENGTH]);
          message[STATE] = 1;
          message[LENGTH] = temp_new_length;
          message[MESSAGE_LAST_INDEX] = introverted_vertexes[i];
        }
      }
  }
}

void send_result()
{
  mySend(&message[MESSAGE_LAST_INDEX], RESULT_SIZE, MPI_INT, source_vertex, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  if(IF_PRINT)
  {
    printf("rank: %d send result\n",rank );
  }
}

void my_collect()
{
  int i;
  result_collect[source_vertex] = source_vertex;
  for(i=0; i<vertexes_number; i++)
  {
    if(i!=source_vertex)
    {
      myRecv(&result_collect[i], RESULT_SIZE, MPI_INT, i, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if(IF_PRINT)
      {
        printf("rank: %d receive result from rank: %d \n",rank, i);
      }
    }
  }
  if(IF_PRINT)
  {
    printf("rank: %d receive result, done\n",rank);
  }

  printf("result of %d :\n" ,vertexes_number);
  int temp;
  for(i=0; i<vertexes_number; i++)
  {
    temp = i;
    do{
      printf("%d   ", temp);
      temp = result_collect[temp];
    }while(temp!=source_vertex);
    printf("%d\n", source_vertex);
  }

}


void my_mpi_execute()
{
    // int loop;

    malloc_data();

    message[STATE] = graph_weight[source_vertex][rank]!=INT_MAX;
    message[LENGTH] = graph_weight[source_vertex][rank];
    message[MESSAGE_LAST_INDEX] = source_vertex;

    if(rank==source_vertex)
    {
        wait_end();
        //end!
        my_collect();

    }
    else if(rank>=vertexes_number)
    {
        wait_end();
    }
    else
    {
        loop = 1;
        while (loop)
        {
            notify_and_recv();
            claculate_and_update();
            myAllreduce(message, &loop, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
        }
        //end
        send_result();
    }
    // free_data();
    if(IF_PRINT)
    {
      printf("rank: %d excute, done\n",rank);
    }
}

void malloc_data()
{
    int i;
    recv_data_temp = (int*)malloc(sizeof(int) * introverted_number*MESSAGE_SIZE);
    recv_data = (int**)malloc(sizeof(int*) *introverted_number);
    result_collect = (int*)malloc(sizeof(int) *introverted_number);

    for(i=0; i<introverted_number; i++)
    {
        recv_data[i] = recv_data_temp + i*MESSAGE_SIZE;
    }
}

void free_data()
{
  free(result_collect);
  free(recv_data_temp);
  free(recv_data);
}

#include "include.h"
#define MESSAGE_SIZE 2
#define MESSAGE_TYPE 0
#define MESSAGE_LENGTH 1
#define MESSAGE_TAG 1


int message[MESSAGE_SIZE];
int **recv_data;
int *recv_data_temp;




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
    double start_time;
    double end_time;
    start_time = MPI_Wtime();

    for(i=0; i<outgoing_number; i++)
    {
        MPI_Isend(message, MESSAGE_SIZE, MPI_INT, outgoing_vertexes[i], MESSAGE_TAG, MPI_COMM_WORLD, &send_request[i]);
    }
    for(i=0; i<introverted_number; i++)
    {
        MPI_Irecv(recv_data[i], MESSAGE_SIZE, MPI_INT, introverted_vertexes[i], MESSAGE_TAG, MPI_COMM_WORLD, &recv_request[i]);
    }
    MPI_Waitall(outgoing_number, send_request, send_status);
    MPI_Waitall(introverted_number, recv_request, recv_status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}


void claculate_and_update()
{
    int i;
    int temp_new_length;

    message[MESSAGE_TYPE] = 0;
    for(i = 0; i<introverted_number; i++)
    {
        if(recv_data[i][MESSAGE_TYPE])
        {
            temp_new_length = recv_data[i][MESSAGE_LENGTH]+graph_weight[introverted_vertexes[i]][rank];
            if( temp_new_length < message[MESSAGE_LENGTH])
            {
                message[MESSAGE_TYPE] = 1;
                message[MESSAGE_LENGTH] = temp_new_length;
                last_index = introverted_vertexes[i];
            }
        }
    }
}



void my_mpi_execute()
{
    malloc_data();

    message[MESSAGE_TYPE] = graph_weight[source_vertex][rank]!=INT_MAX;
    message[MESSAGE_LENGTH] = graph_weight[source_vertex][rank];
    last_index = graph_weight[source_vertex][rank]==INT_MAX?-1:source_vertex;

    if(rank==source_vertex || rank>=vertexes_number)
    {
        wait_end();
    }
    else
    {
        int loop = 1;
        while (loop)
        {
            notify_and_recv();
            claculate_and_update();
            myAllreduce(message, &loop, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
        }
    }
    my_collect_and_send();
    free_data();
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


    for(i=0; i<introverted_number; i++)
    {
        recv_data[i] = recv_data_temp + i*MESSAGE_SIZE;
    }
}

void free_data()
{
    free(recv_data_temp);
    free(recv_data);
}

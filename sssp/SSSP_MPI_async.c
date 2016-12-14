#include "include.h"

#define MESSAGE_TYPE_WHITE 1
#define MESSAGE_TYPE_BLACK 2

#define MESSAGE_TYPE_UPDATE 16
#define MESSAGE_TYPE_TERMINATE 8

#define MESSAGE_TYPE 0
#define MESSAGE_LENGTH 1
#define MESSAGE_SIZE 2
#define MESSAGE_TAG 1

#define RING_INITIAL 0
#define BLACK_ARRIVAL 1
#define WHITE_ARRIVAL 2
#define BLACK_SENT 3
#define WHITE_SENT 4

#define RING_STATE_SIZE 1
#define RING_STATE_TAG 2

int message[MESSAGE_SIZE];
int received[MESSAGE_SIZE];


void malloc_data();
void free_data();
void my_mpi_execute();

void message_send(int destination)
{
    mySend(message, MESSAGE_SIZE, MPI_INT, destination, MESSAGE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
void message_receive()
{
    MPI_Status status;
    myRecv(received, MESSAGE_SIZE, MPI_INT, MPI_ANY_SOURCE, MESSAGE_TAG, MPI_COMM_WORLD, &status);
    return status.MPI_SOURCE;
}
int main(int argc,char *argv[])
{
    my_mpi_init(argc, argv);
    my_mpi_execute();
    my_mpi_finalize();
    return 0;
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
void nonSourceVertexCompute()
{

    int ring_state = RING_INITIAL;
    int received_state;
    do {
        int source = message_receive();
        if(received[MESSAGE_TYPE]==MESSAGE_TYPE_UPDATE)
        {
            MPI_Isend(&ring_state, RING_STATE_SIZE, MPI_INT, source, RING_STATE_TAG, MPI_COMM_WORLD, &send_request[0]);

            int temp_new_length = received[MESSAGE_LENGTH]+graph_weight[source][rank];
            if( temp_new_length < message[MESSAGE_LENGTH])
            {
                message[MESSAGE_LENGTH] = temp_new_length;
                message[MESSAGE_TYPE] = MESSAGE_TYPE_UPDATE;
                last_index = sender_rank;

                notify_and_recv();
                MPI_Wait(&send_request[0], &send_status[0]);
                if(ring_state==WHITE_SENT)
                {
                    ring_state = RING_INITIAL;
                }
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
        else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_TERMINATE)
        {
            if(ring_state!=WHITE_SENT)
            {
                printf("rank:%d error to end, local work is not done but controller node send terminate information!\n",rank );
            }
            break;
        }
    } while(1);
}

void sourceVertexCompute()
{
    int i;
    int next_rank = getNextNodeRank();
    do {
        message[MESSAGE_TYPE] = MESSAGE_TYPE_WHITE;
        message_send(next_rank);
        message_receive();
        if(received[MESSAGE_TYPE]==MESSAGE_TYPE_WHITE)
        {
            message[MESSAGE_TYPE] = MESSAGE_TYPE_TERMINATE;
            for(i=0; i<vertexes_number; i++)
            {
                if(i!=source_vertex)
                {
                    message_send(i);
                }
            }
            break;
        }else if(received[MESSAGE_TYPE]!=MESSAGE_TYPE_BLACK)
        {
            printf("error to receive unrecognized message");
        }
    } while(1);
}

void my_mpi_execute()
{
    malloc_data();
    message[MESSAGE_TYPE] = 0;
    message[MESSAGE_LENGTH] = graph_weight[source_vertex][rank];
    last_index = graph_weight[source_vertex][rank]==INT_MAX?-1:source_vertex;
    if(rank!=source_vertex)
    {
        nonSourceVertexCompute();
    }
    else
    {
        sourceVertexCompute();
    }

    my_collect_and_send();
    free_data();
}

void malloc_data()
{}

void free_data()
{}

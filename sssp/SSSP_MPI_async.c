#include "include.h"

#define MESSAGE_TYPE_NO_MESSAGE 0
#define MESSAGE_TYPE_WHITE 1
#define MESSAGE_TYPE_BLACK 2

#define MESSAGE_TYPE_UPDATE 16
#define MESSAGE_TYPE_TERMINATE 8
#define MESSAGE_TYPE_ASK_FOR_STATE 32

#define MESSAGE_TYPE 0
#define MESSAGE_LENGTH 1
#define MESSAGE_SIZE 2
#define MESSAGE_TAG 1

#define RING_INITIAL 0
#define BLACK_SENT 1
#define WHITE_SENT 2

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

int message_receive()
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

void update_message(int exception)
{
    int i;
    for (i = 0; i < outgoing_number; ++i)
    {
        if(outgoing_vertexes[i]!=exception)
        {
            message_send(outgoing_vertexes[i]);
            if(DEBUG)
            {
                printf("rank: %d , send update message to the %d\n", rank, outgoing_vertexes[i]);
            }
        }
    }
}

void nonSourceVertexCompute()
{

    int ring_state = RING_INITIAL;
    int next_rank = getNextNodeRank();
    int is_reactive = 0;
    int received_is_reactive;
    int temp_index;

    if(message[MESSAGE_TYPE]==MESSAGE_TYPE_UPDATE)
    {
        if(DEBUG)
        {
            printf("rank: %d try to send first update message! \n",rank);
        }
        update_message(source_vertex);
        if(DEBUG)
        {
            printf("rank: %d send first update message, done  \n",rank);
        }
    }

    int i = 0;
    do {
        i++;
        int sender_rank = message_receive();
        if(DEBUG)
        {
            printf("rank: %d received message from %d type: %d \n", rank, sender_rank, received[MESSAGE_TYPE]);
        }
        if(received[MESSAGE_TYPE]==MESSAGE_TYPE_UPDATE)
        {
            int temp_new_length = received[MESSAGE_LENGTH]+graph_weight[sender_rank][rank];
            if( temp_new_length < message[MESSAGE_LENGTH])
            {
                if(DEBUG)
                {
                    printf("rank: %d , update!!\n", rank );
                }
                message[MESSAGE_LENGTH] = temp_new_length;
                message[MESSAGE_TYPE] = MESSAGE_TYPE_UPDATE;
                last_index = sender_rank;
                if(ring_state==WHITE_SENT)
                {
                    is_reactive = 1;
                    ring_state = RING_INITIAL;
                }update_message(sender_rank);
            }
        }else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_ASK_FOR_STATE)
        {
            mySend(&is_reactive, 1, MPI_INT, sender_rank, RING_STATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_WHITE)
        {
            ring_state = WHITE_SENT;
            is_reactive = 0;
            message[MESSAGE_TYPE] = MESSAGE_TYPE_ASK_FOR_STATE;
            /**ask before vertex if there is anyone has been reactive**/
            for(temp_index=0; temp_index<outgoing_number; temp_index++)
            {
                if(!isAfterVertex(rank, outgoing_vertexes[temp_index]))break;
                if(DEBUG)
                {
                    printf("rank: %d ,verify to the outgoing : %d\n", rank , outgoing_vertexes[temp_index]);
                }
                mySendrecv(message, MESSAGE_SIZE, MPI_INT, outgoing_vertexes[temp_index], MESSAGE_TAG,
                           &received_is_reactive, 1, MPI_INT, outgoing_vertexes[temp_index], RING_STATE_TAG,
                           MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(received_is_reactive)
                {
                    message[MESSAGE_TYPE] = MESSAGE_TYPE_BLACK;
                    ring_state = BLACK_SENT;
                    break;
                }
            }
            if(message[MESSAGE_TYPE]!=MESSAGE_TYPE_BLACK)
            {
                message[MESSAGE_TYPE] = MESSAGE_TYPE_WHITE;
            }
            if(DEBUG)
            {
                printf("rank: %d , send white or black message to next: %d\n", rank , next_rank);
            }
            message_send(next_rank);
        }
        else if(received[MESSAGE_TYPE]==MESSAGE_TYPE_BLACK)
        {
            is_reactive = 0;
            ring_state = BLACK_SENT;
            message[MESSAGE_TYPE] = MESSAGE_TYPE_BLACK;
            if(DEBUG)
            {
                printf("rank: %d , send black message to next: %d\n", rank , next_rank);
            }
            message_send(next_rank);
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
    message[MESSAGE_TYPE] = graph_weight[source_vertex][rank]==INT_MAX?MESSAGE_TYPE_NO_MESSAGE:MESSAGE_TYPE_UPDATE;
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

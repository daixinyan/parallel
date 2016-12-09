#include "include.h"

int   vertexes_number;
int   edges_half_number;

int   threads_number;
char* input_file_name;
char* output_file_name;
int   source_vertex;

int** graph_weight;
int*  outgoing_vertexes;
int   outgoing_number;
int*  introverted_vertexes;
int   introverted_number;

MPI_Request *send_request;
MPI_Request *recv_request;
MPI_Status  *send_status;
MPI_Status  *recv_status;

/**record time**/
double total_start_time;
double total_end_time;
/**start struct time**/
double communication_time = .0;
double compution_time = .0;
double barrier_time = .0;
double total_time = .0;
/**end struct time**/
int size, rank, actual_size;
void readGraph();
void init_malloc();

void init_neibors()
{
    int i;
    if(rank==source_vertex) return;

    outgoing_number = 0;
    introverted_number = 0;
    for ( i = 0; i < vertexes_number; i++)
    {
        if(i!=source_vertex && i!=rank && graph_weight[rank][i]!=INT_MAX)
        {
            outgoing_vertexes[outgoing_number++] = i;
        }
        if(i!=source_vertex && i!=rank && graph_weight[i][rank]!=INT_MAX)
        {
            introverted_vertexes[introverted_number++] = i;
        }
    }

}

void init_malloc()
{
    int i,j;
    int* temp_onedim_array = (int*)malloc(sizeof(int)*vertexes_number*vertexes_number);
    outgoing_vertexes = (int*)malloc(sizeof(int)*vertexes_number);
    introverted_vertexes = (int*)malloc(sizeof(int)*vertexes_number);
    graph_weight = (int**)malloc(sizeof(int*)*vertexes_number);
    for(i=0; i<vertexes_number; i++)
    {
        graph_weight[i] = temp_onedim_array + i*vertexes_number;
    }
    for (i = 0; i < vertexes_number; i++)
    {
        for (j = 0; j < vertexes_number; j++)
        {
            graph_weight[i][j] = INT_MAX;
        }
    }

    send_request = malloc(sizeof(MPI_Request)*vertexes_number);
    recv_request = malloc(sizeof(MPI_Request)*vertexes_number);
    send_status = malloc(sizeof(MPI_Status)*vertexes_number);
    recv_status = malloc(sizeof(MPI_Status)*vertexes_number);
}

void my_global_free()
{
  free(outgoing_vertexes);
  free(introverted_vertexes);
  free(graph_weight[0]);
  free(graph_weight);
  free(send_request);
  free(send_status);
  free(recv_request);
  free(recv_status);
}

void my_mpi_finalize()
{
    // my_global_free();
    total_end_time = MPI_Wtime();
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time;

    if(PRINT_TIME)
    {
        printf("rank: %d\n total_time: %f\n communication_time: %f\n compution_time: %f\n",
               rank, total_time, communication_time, compution_time );
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

}


void readGraph()
{
    FILE *fp;
    int i;
    int from_index,to_index,distance;

    if((fp = fopen(input_file_name,"r"))==NULL)
    {
        printf("error to open %s!", input_file_name);
        exit(1);
    }
    fscanf(fp, "%d", &vertexes_number);
    fscanf(fp, "%d", &edges_half_number);

    init_malloc();

    for (i = 0; i < edges_half_number; i++)
    {
        fscanf(fp, "%d", &from_index);
        fscanf(fp, "%d", &to_index);
        fscanf(fp, "%d", &distance);
        graph_weight[to_index-1][from_index-1] = distance;
        graph_weight[from_index-1][to_index-1] = distance;
    }
    if(IF_PRINT)
    {
        printf("rank: %d, read data, done\n", rank);
    }
    init_neibors();
}

void my_mpi_init(int argc,char *argv[])
{
    /**init mpi**/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    my_init(argc, argv);
}

void  my_init(int argc,char *argv[])
{
    total_start_time = MPI_Wtime();

    if(argc<4)
    {
        printf("no enough args\n");
        exit(1);
    }
    else
    {
        threads_number = atoi(argv[1]);
        input_file_name = argv[2];
        output_file_name = argv[3];
        source_vertex = atoi(argv[4]);
    }

    readGraph();
    int i;
    for(i = 0; i<outgoing_number; i++)
    {
      printf("rank: %d %d %d\n",rank, i, outgoing_vertexes[i]);
    }

}

void myAllreduce(const void* sendbuf, void* recv_data, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Allreduce(sendbuf, recv_data, count, datatype, op, comm);
    end_time = MPI_Wtime();
    barrier_time += end_time-start_time;
}

void mySendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,int source, int recvtag,MPI_Comm comm, MPI_Status *status)
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Sendrecv(sendbuf, sendcount, sendtype,dest, sendtag,
                 recvbuf, recvcount, recvtype,source, recvtag, comm,  status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

void myRecv(void *buf, int count, MPI_Datatype type,int source, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

void mySend(const void *buf, int count, MPI_Datatype type,int dest, int tag,MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

void myWaitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[])
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Waitall(count, array_of_requests, array_of_statuses);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

void myWait(MPI_Request *request,MPI_Status *status)
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Wait(request, status);
    end_time = MPI_Wtime();
    communication_time += end_time-start_time;
}

void myIsend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)
{
double start_time;
double end_time;
start_time = MPI_Wtime();
MPI_Isend(buf, count, datatype, dest, tag, comm, request);
end_time = MPI_Wtime();
communication_time += end_time-start_time;
}

void myIrecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)
{
double start_time;
double end_time;
start_time = MPI_Wtime();
MPI_Irecv(buf, count, datatype, source, tag, comm, request);
end_time = MPI_Wtime();
communication_time += end_time-start_time;
}

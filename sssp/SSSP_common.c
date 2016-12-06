#include "include.h"

int   vertexes_number;
int   edges_half_number;

int   threads_number;
char* input_file_name;
char* output_file_name;
int   source_vertex;

int** graph_weight;

/**record time**/
double total_start_time;
double total_end_time;
/**start struct time**/
double communication_time = .0;
double compution_time = .0;
double total_time = .0;
/**end struct time**/
int size, rank, actual_size;
void readGraph();
void init_graph();


void init_graph()
{
    int i,j;
    int* temp_onedim_array = (int*)malloc(sizeof(int)*vertexes_number*vertexes_number);
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
}


void my_finalize()
{
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

    init_graph();

    for (i = 0; i < edges_half_number; i++)
    {
        fscanf(fp, "%d", &from_index);
        fscanf(fp, "%d", &to_index);
        fscanf(fp, "%d", &distance);
        if(IF_PRINT && rank==0)
        {
          printf("rank: %d, distance:%d, i:%d, j:%d\n",rank,distance,from_index, to_index);
        }
        // graph_weight[to_index][from_index] = distance;
        // graph_weight[from_index][to_index] = distance;
    }
}


void  my_init(int argc,char *argv[])
{
    /**init mpi**/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    total_start_time = MPI_Wtime();

    if(argc<3)
    {
        printf("no enough args\n");
        exit(1);
    }
    else
    {
        printf("init args\n");
        threads_number = atoi(argv[1]);
        input_file_name = argv[2];
        output_file_name = argv[3];

        source_vertex = argc==3? 0:atoi(argv[4]);
    }
    if(rank==0)readGraph();
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

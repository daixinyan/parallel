#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define PRINT_MESSAGE 0
#define PRINT_RESULT 0
#define PRINT_TIME 0

#define MAX_TRANSFER 512

#define TAG_NUMBER_FROM_UP     1
#define TAG_NUMBER_FROM_DOWN   2
#define TAG_NOSWAPED_FROM_NODES   3
#define TAG_NOSWAPED_FROM_CENTER 4
#define CONTROLL_NODE 0
int size,rank;
int actual_size;
char process_name[MPI_MAX_PROCESSOR_NAME];
int  name_len;
int total_int_num;
char* input_file;
char* output_file;

int current_process_int_num;
int current_process_start_index;
int current_process_end_index;
int offset;
int* data;
int  temp;

int* temp_resceive_buf;
int* temp_send_buf;
int* temp_sorted_buf;
int int_num_transafer = 1;

int up_transfer ;
int down_transfer ;

double communication_time = .0;

double read_time = .0;
double write_time = .0;
double compution_time = .0;
double total_time = .0;


void printArray(int *a, int start, int length);
void quickSort( int[], int, int);

int partition( int[], int, int);
int  my_compare_swap(int* a, int i, int j);
void my_swap(int* a, int i, int j);

void my_read();
void my_init();
void my_write();

void my_sort();
int  sendToUp();
int  sendToDown();
void mySendrecv(
                const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int source, int recvtag,
                MPI_Comm comm, MPI_Status *status
                );
void myRecv(void *buf, int count, MPI_Datatype type,
                int source, int tag,
                MPI_Comm comm, MPI_Status *status );
void mySend(const void *buf, int count, MPI_Datatype type,
                int dest, int tag,
                MPI_Comm comm, MPI_Status *status );



int main(int argc,char *argv[])
{
    double total_start_time;
    double total_end_time;

    double start_time;
    double end_time;


        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Get_processor_name(process_name,&name_len);

    total_start_time = MPI_Wtime();


        if(argc<3){
            total_int_num = 4;
            input_file = "testcase/testcase1";
            output_file = "test_out_0";
            if(PRINT_MESSAGE)
                printf("too less parameters.from the node %d,\n total size is %d\n", rank,size);
        }else
        {
            total_int_num = atoi(argv[1]);
            input_file = argv[2];
            output_file = argv[3];
            if(PRINT_MESSAGE)
                printf("complete parameters get. from the node %d,\n total size is %d\n", rank,size);
        }
        if(PRINT_MESSAGE)
        {
            printf("number: %d, input: %s, output: %s\n", total_int_num, input_file, output_file);
        }

        my_init();


    start_time = MPI_Wtime();
        my_read();
    end_time = MPI_Wtime();
    read_time = end_time-start_time;



    if(size>1)
    {
         my_sort();
    }else
    {
        quickSort(data,0,current_process_int_num-1);
    }




    start_time = MPI_Wtime();
        my_write();
    end_time = MPI_Wtime();
    write_time = end_time-start_time;

    total_end_time = end_time;
    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time - read_time - write_time;


    if(PRINT_MESSAGE)
    {
         printArray(data,0,current_process_int_num);
    }
    if(PRINT_TIME)
    {
        printf("total_time: %f\n communication_time: %f\n compution_time: %f\n read_time: %f\n write_time: %f\n",total_time, communication_time, compution_time, read_time, write_time );

    }


    MPI_Finalize();
}


void my_write()
{

    MPI_File mpi_file;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, output_file,
                  MPI_MODE_CREATE|MPI_MODE_WRONLY,
                   MPI_INFO_NULL,&mpi_file);

    if(current_process_int_num!=0)
    {
        MPI_File_write_at(mpi_file,
                      offset,data,current_process_int_num,
                      MPI_INT,&status);
        if(PRINT_MESSAGE)
            printf("write down. %d\n", rank);
    }
    MPI_File_close(&mpi_file);

}



void my_read()
{

	int i;
    MPI_File mpi_file;
    MPI_Status status;

    MPI_File_open(MPI_COMM_WORLD, input_file, MPI_MODE_RDONLY, MPI_INFO_NULL, &mpi_file);
    if(PRINT_MESSAGE) printf("------------open success at rank %d\n",rank);
    if(current_process_int_num!=0)
    {
        MPI_File_seek(mpi_file, offset, MPI_SEEK_SET);
        MPI_File_read(mpi_file, data, current_process_int_num, MPI_INT, &status);
        if(PRINT_MESSAGE) printf("------------read done at rank %d\n",rank);
    }

    MPI_File_close(&mpi_file);//after open, fh has the communicator info
    if(PRINT_MESSAGE)
        {
            printf("data start------------------------\n");
            printArray(data,0,current_process_int_num);
            printf("data end------------------------\n\n");
        }
}

/**calculate allocate integers start index and end index.**/
void my_init()
{
    int reminder,quotient;
    reminder = total_int_num%size;
    quotient = total_int_num/size;
    if(rank>=reminder)// rank>0 reminder>0
    {
        current_process_start_index = quotient*rank + reminder;
        current_process_int_num = quotient;
    }else
    {
        current_process_start_index = (quotient+1)*rank;
        current_process_int_num = quotient+1;
    }
    current_process_end_index = current_process_start_index+current_process_int_num-1;
    if(PRINT_MESSAGE)
    {
        printf("reminder: %d, quotient: %d, start_index: %d, int_number: %d, node_index: %d\n",
               reminder, quotient, current_process_start_index,current_process_int_num,rank);
    }

    actual_size = quotient==0?reminder:size;
    int_num_transafer = quotient<20?1:quotient/16;
    int_num_transafer = (int_num_transafer>MAX_TRANSFER)?MAX_TRANSFER:int_num_transafer;
    int_num_transafer = quotient<2?1:(int)(quotient/2+1);

    data = (int*)(malloc(current_process_int_num*sizeof(int)));
    temp_resceive_buf = (int*)(malloc(int_num_transafer*sizeof(int)));
    temp_sorted_buf = (int*)(malloc(int_num_transafer*sizeof(int)));
    offset = current_process_start_index*sizeof(int);
}

void my_sort()
{
    int swaped = 0;
    int i,j,k;

    int loop = size*2;
    if(current_process_int_num==0) return;
    quickSort(data,0,current_process_int_num-1);
    for(i=0; i<loop; i++)
    {
        if(PRINT_MESSAGE)
        {
            printf("data start------------------------\n");
            printArray(data,0,current_process_int_num);
            printf("data end------------------------\n\n");
        }
        if(rank&1)
        {
            swaped = swaped | sendToDown();
            swaped = swaped | sendToUp();
        }else
        {
            swaped = swaped | sendToUp();
            swaped = swaped | sendToDown();
        }

        if(rank!=CONTROLL_NODE)
        {
            if(PRINT_MESSAGE)printf("at node %d, swaped %d, send to center.\n",rank, swaped);
            mySendrecv(
                           &swaped, 1, MPI_INT,
                           CONTROLL_NODE,TAG_NOSWAPED_FROM_NODES,
                           &temp, 1, MPI_INT ,
                           CONTROLL_NODE, TAG_NOSWAPED_FROM_CENTER,
                           MPI_COMM_WORLD, MPI_STATUS_IGNORE
                       );
            if(PRINT_MESSAGE)printf("at node %d, swaped %d, received from center.\n",rank, swaped);
            if(temp==0)
            {
                break;
            }
        }else
        {
            if(PRINT_MESSAGE)printf("at node %d, swaped %d, received from nodes\n",rank, swaped);
            for(j=1;j<actual_size;j++)
            {
                myRecv(&temp, 1, MPI_INT, j, TAG_NOSWAPED_FROM_NODES,MPI_COMM_WORLD, MPI_STATUS_IGNORE );
                swaped = swaped | temp;
            }
            if(PRINT_MESSAGE)printf("at node %d, swaped %d, send to nodes.\n",rank, swaped);
            for(j=1;j<actual_size;j++)
            {
                mySend(&swaped, 1, MPI_INT, j, TAG_NOSWAPED_FROM_CENTER,MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            }
            if(PRINT_MESSAGE)printf("at node %d, swaped %d, send to nodes,done.\n",rank, swaped);
            if(!swaped)
            {
                break;
            }
        }

    }
}

int sendToUp()
{
    int i,j,k;
    if(rank==actual_size-1)
    {
        return 0;
    }
    mySendrecv(
                        &data[current_process_int_num-int_num_transafer], int_num_transafer, MPI_INT,
                        rank+1,TAG_NUMBER_FROM_DOWN,
                        temp_resceive_buf, int_num_transafer, MPI_INT ,
                        rank+1, TAG_NUMBER_FROM_UP,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE
                );
    if(PRINT_MESSAGE)
    {
        printf("at node %d  communication with %d\n",rank, rank+1);
        printArray(temp_resceive_buf,0,int_num_transafer);
    }
    i = j = k = 0;
    for(; i<int_num_transafer; i++)
    {
        if(data[j+current_process_int_num-int_num_transafer]>temp_resceive_buf[k])
        {
            temp_sorted_buf[i] = temp_resceive_buf[k];
            k++;
        }else
        {
            temp_sorted_buf[i] = data[j+current_process_int_num-int_num_transafer];
            j++;
        }
    }
    if(PRINT_MESSAGE)
    {
        printf("at node %d with %d temp sorted buffer:\n",rank, rank+1);
        printArray(temp_sorted_buf,0,int_num_transafer);
    }
    if(k==0)
    {
        return 0;
    }

    i = current_process_int_num-1;
    j = current_process_int_num-int_num_transafer-1;
    k = int_num_transafer-1;
    for(;i>=0&&k>=0; i--)
    {
        if(data[j]>temp_sorted_buf[k] && j>=0)
        {
            data[i] = data[j];
            j--;

        }else
        {
            data[i] = temp_sorted_buf[k];
            k--;
        }

    }
    if(PRINT_MESSAGE)
        {
            printf("     data start------------------------\n\n");
            printArray(data,0,current_process_int_num);
            printf("     data end------------------------\n\n");
        }
    return 1;
}
int sendToDown()
{
    int i,j,k,has_swaped;

    if(rank==0)
    {
        return 0;
    }
    mySendrecv(
                           data, int_num_transafer, MPI_INT,
                           rank-1, TAG_NUMBER_FROM_UP,
                           temp_resceive_buf, int_num_transafer, MPI_INT ,
                           rank-1, TAG_NUMBER_FROM_DOWN,
                           MPI_COMM_WORLD, MPI_STATUS_IGNORE
                );
    if(PRINT_MESSAGE)
    {
        printf("at node %d  communication with %d\n",rank, rank-1);
        printArray(temp_resceive_buf,0,int_num_transafer);
    }

    i = j = k = int_num_transafer-1;
    for(; i>=0; i--)
    {
        if(data[j]>temp_resceive_buf[k])
        {
            temp_sorted_buf[i] = data[j];
            j--;
        }else
        {
            temp_sorted_buf[i] = temp_resceive_buf[k];
            k--;
        }
    }
    if(PRINT_MESSAGE)
    {
        printf("at node %d with %d temp sorted buffer:\n",rank, rank-1);
        printArray(temp_sorted_buf,0,int_num_transafer);
    }
    if(k==int_num_transafer-1)
    {
        return 0;
    }
    i = 0;
    j = int_num_transafer;
    k = 0;
    for(;i<current_process_int_num && k<int_num_transafer; i++)
    {
        if(j<current_process_int_num && data[j]<temp_sorted_buf[k])
        {
            data[i] = data[j];
            j++;
        }else
        {
            data[i] = temp_sorted_buf[k];
            k++;
        }

    }
     if(PRINT_MESSAGE)
        {
            printf("     data start------------------------\n\n");
            printArray(data,0,current_process_int_num);
            printf("     data end------------------------\n\n");
        }
    return 1;
}

void mySendrecv(
                const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int source, int recvtag,
                MPI_Comm comm, MPI_Status *status
                )
                {
                    double start_time;
                    double end_time;
                    start_time = MPI_Wtime();
                    MPI_Sendrecv(sendbuf, sendcount, sendtype,dest, sendtag,
                                 recvbuf, recvcount, recvtype,source, recvtag,
                                 comm,  status);
                    end_time = MPI_Wtime();
                    communication_time += end_time-start_time;
                }
void myRecv(void *buf, int count, MPI_Datatype type,
                int source, int tag,
                MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    communication_time += end_time-start_time;
}
void mySend(const void *buf, int count, MPI_Datatype type,
                int dest, int tag,
                MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);

    communication_time += end_time-start_time;
}


void printArray(int *a, int start, int length)
{
    int i;
    for(i=0; i<length; i++)
    {
        printf("%d : %d : 0x%X  %d \n",
               rank,current_process_start_index+start+i,
               a[i+start],a[i+start]);
    }
}

int my_compare_swap(int* a, int i, int j)
{
    if(a[i]>a[j])
    {
        my_swap(a,i,j);
        return 1;
    }
    return 0;
}

void my_swap(int* a, int i, int j)
{
    int temp = a[i];
    a[i] = a[j];
    a[j] = temp;
}

void quickSort( int a[], int l, int r)
{
    int j;
    if( l < r )
    {
        j = partition( a, l, r);
        quickSort( a, l, j-1);
        quickSort( a, j+1, r);
    }

}



int partition( int a[], int l, int r) {
   int pivot, i, j, t;
   int p = rand()%(r-l+1)+l;
   pivot = a[p];
   my_swap(a,l,p);
   i = l; j = r+1;

   while( 1)
   {
   	do ++i; while( a[i] <= pivot && i <= r );
   	do --j; while( a[j] > pivot );
   	if( i >= j ) break;
   	t = a[i]; a[i] = a[j]; a[j] = t;
   }
   t = a[l]; a[l] = a[j]; a[j] = t;
   return j;
}

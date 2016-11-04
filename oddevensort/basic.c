#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define PRINT_MESSAGE 1
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

double communication_time = .0;

double read_time = .0;
double write_time = .0;
double compution_time = .0;
double total_time = .0;


void my_read();
void my_init();
void odd_even_sort(int* a, int head, int tail);
void my_write();
void printArray(int *a, int start, int length);
int  my_compare_swap(int* a, int i, int j);
void my_swap(int* a, int i, int j);
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
int myRecv(void *buf, int count, MPI_Datatype type,
                int source, int tag,
                MPI_Comm comm, MPI_Status *status );
int mySend(const void *buf, int count, MPI_Datatype type,
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
        if(PRINT_MESSAGE)
        {
            printf("size > 1, normal sort.\n");
        }
        my_sort();
    }

    else
    {
        if(PRINT_MESSAGE)
        {
            printf("size == 1, local sort.\n");
        }
        odd_even_sort(data,0,total_int_num);
    }




    start_time = MPI_Wtime();
        my_write();
    total_end_time = end_time = MPI_Wtime();
    write_time = end_time-start_time;

    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time - read_time - write_time;

    printf("total_time: %f\n communication_time: %f\n compution_time: %f\n read_time: %f\n write_time: %f\n",total_time, communication_time, compution_time, read_time, write_time );


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
            printf("write down. %d", rank);
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
    data = (int*)(malloc(current_process_int_num*sizeof(int)));
    offset = current_process_start_index*sizeof(int);
}

void my_sort()
{

    if(current_process_int_num==0) return;
    int i,j,k;
    int start;

    int swaped = 0;
    int send_to_up,send_to_down ;

    for(i=1; i<1+total_int_num ; i++)
    {

        swaped = 0;
        if(
            (
               ( (i&1) && (current_process_start_index&1) ) //0 12 34 56 78 9
               ||
               ( !(i&1) &&!(current_process_start_index&1) )//01 23 45 67 89
             )
          )
          {
              start=0;
              send_to_down = 0;
          }
          else
          {
              start = 1;
              send_to_down = rank!=0;
          }


        if(
           ( (i&1) && (current_process_end_index&1) ) //0 12 34 56 78 9
           ||
           ( !(i&1) &&!(current_process_end_index&1) )//01 23 45 67 89
          )
          {
              send_to_up = (rank!=(size-1)) && current_process_end_index!=(total_int_num-1);
          }
          else
          {
              send_to_up = 0;
          }


        for(k=start;k<current_process_int_num-1;k+=2)
        {
            swaped = swaped | my_compare_swap(data,k,k+1);
        }
        for(k=start?0:1;k<current_process_int_num-1;k+=2)
        {
            swaped = swaped | data[k]>data[k+1];
            if(swaped)
            {
                break;
            }
        }

        if(rank&1)
        {

            if(send_to_down)
            {
                swaped = swaped | sendToDown();
            }
            if(send_to_up)
            {
                swaped = swaped | sendToUp();
            }
        }else
        {
            if(send_to_up)
            {
                swaped = swaped | sendToUp();
            }
            if(send_to_down)
            {
                swaped = swaped | sendToDown();
            }
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
    mySendrecv(
                           &data[current_process_int_num-1], 1, MPI_INT,
                           rank+1,TAG_NUMBER_FROM_DOWN,
                           &temp, 1, MPI_INT ,
                           rank+1, TAG_NUMBER_FROM_UP,
                           MPI_COMM_WORLD, MPI_STATUS_IGNORE
                );
    if(PRINT_MESSAGE)
    {
        printf("at node %d  communication with %d\n,get message %d\n",
               rank, rank+1, temp);
    }
    if(temp<data[current_process_int_num-1])
    {
        data[current_process_int_num-1] = temp;
        return 1;
    }
    return 0;

}
int sendToDown()
{
    mySendrecv(
                           &data[0], 1, MPI_INT,
                           rank-1, TAG_NUMBER_FROM_UP,
                           &temp, 1, MPI_INT ,
                           rank-1, TAG_NUMBER_FROM_DOWN,
                           MPI_COMM_WORLD, MPI_STATUS_IGNORE
                );
    if(PRINT_MESSAGE)
    {
        printf("at node %d  communication with %d\nget message %d\n",
               rank, rank-1,temp);
    }
    if(temp>data[0])
    {
        data[0] = temp;
        return 1;
    }
    return 0;
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
int myRecv(void *buf, int count, MPI_Datatype type,
                int source, int tag,
                MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Recv(buf,count,type,source,tag,comm,status);
    communication_time += end_time-start_time;
}
int mySend(const void *buf, int count, MPI_Datatype type,
                int dest, int tag,
                MPI_Comm comm, MPI_Status *status )
{
    double start_time;
    double end_time;
    start_time = MPI_Wtime();
    MPI_Send(buf,count,type,dest,tag,comm);

    communication_time += end_time-start_time;
}

void odd_even_sort(int* a, int head, int tail)
{
    int i,j;
    int loop = 1;
    int start;

    int odd_pass = (tail-head)/2;
    int even_pass = (tail-head-1)/2;
    for(i=0; i<tail-head && loop; i++)
    {

        if(i&1)/*odd pass*/
        {
          if(PRINT_MESSAGE)
          {
              printf("odd pass %d\n",i);
          }
          /*if length is odd : 0 to length-1 or else 0 to length*/
          for( start=0; start<odd_pass; start++)
          {
              my_compare_swap(a,start*2+head,start*2+1+head);
          }
        }else/*even pass*/
        {
            if(PRINT_MESSAGE)
            {
              printf("even pass %d\n",i);
            }
            for( start=0; start<even_pass; start++)
            {
                my_compare_swap(a,start*2+1+head,start*2+2+head);
            }
        }
        loop = 0;
        for(j=1; j<tail-head; j++)
        {
            if(a[j-1]>a[j])
            {
                loop = 1;
                break;
            }
        }
    }
}

void printArray(int *a, int start, int length)
{
    int i;
    for(i=0; i<length; i++)
    {
        printf("from node %d , number is 0x%X  %d \n",
               rank, data[i+start],data[i+start]);
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


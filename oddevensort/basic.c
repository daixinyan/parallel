#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define PRINT_MESSAGE 0
#define PRINT_TIME 1
#define TAG_NUMBER_FROM_UP     1
#define TAG_NUMBER_FROM_DOWN   2
#define TAG_NOSWAPED_FROM_NODES   3
#define TAG_NOSWAPED_FROM_CENTER 4
#define CONTROLL_NODE 0

/**start struct meta**/
    int size,rank;
    int actual_size;/*when the size=10, only 7 elements to allocate, so actual_size=7*/
    char process_name[MPI_MAX_PROCESSOR_NAME];
    int  name_len;

    int total_int_num;/*number of elements*/
    char* input_file; /* input file_name*/
    char* output_file;/*out_put file_name*/

    int current_process_int_num;    /*number of current process's elements*/
    int current_process_start_index;/*actually equal to the number of left process elements*/
    int current_process_end_index;/**/
    int offset;/*offset in file*/
    int* data;/*current process data*/
    int  temp;/*for receive single integer data.*/
/**end struct**/


/**start struct time**/
double communication_time = .0;
double read_time = .0;
double write_time = .0;
double compution_time = .0;
double total_time = .0;
/**end struct time**/

/*init the basic data in struct meta*/
/**calculate allocate integers start index and end index.**/
void my_init();

/**seek and read from remote file into (int*)data.**/
void my_read();
/**seek and write (int*)data to remote file.**/
void my_write();

/**local ood-even-sort**/
void odd_even_sort(int* a, int head, int tail);

/*
 * start a for loop to start even and odd pass.
 ** 1. compare and swap local data.
 ** 2. determine if there is a need to send a element to the left and left process
 ** 3. if there is a need to send a element to the left process, send and receive, then compare and keep the bigger one
 ** 4. if there is a need to send a element to the right process, send and receive, then compare and keep the smaller one
 ** 5. if current process is not controller: inform the 0th process whether any swap happened in previous steps,
 **                                          then according the received message to decide if or not continue the loop
 ** 6.   if current process is controller: after receiving message from all others,determine if or not continue the loop and  inform others.
 */
void my_sort();
/*
 * precondition: rank<actual_size(current process is not the  rightmost process)
 * send the last element to right(rank+1) process,
 * at the same time reveice one.
 * compare the sended one and received one, keey the smaller one at end of data
 */
int  sendToUp();

/*
 * precondition: rank>0 (current process is not the leftmost process)
 * send the first element to left(rank-1) process,
 * at the same time reveice one.
 * compare the sended one and received one, keey the bigger one at start of data
 */
int  sendToDown();


/**print to console**/
void printArray(int *a, int start, int length);
/****/
int  my_compare_swap(int* a, int i, int j);
/****/
void my_swap(int* a, int i, int j);


/**@see MPI_Sendrecv, add comsumption time to (double)communication_time**/
void mySendrecv(
                const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int source, int recvtag,
                MPI_Comm comm, MPI_Status *status
                );

/**@see MPI_Recv, add comsumption time to (double)communication_time**/
void myRecv(void *buf, int count, MPI_Datatype type,
                int source, int tag,
                MPI_Comm comm, MPI_Status *status );

/**@see MPI_Send, add comsumption time to (double)communication_time**/
void mySend(const void *buf, int count, MPI_Datatype type,
                int dest, int tag,
                MPI_Comm comm, MPI_Status *status );



int main(int argc,char *argv[])
{

    /**record time**/
    double total_start_time;
    double total_end_time;

    double start_time;
    double end_time;


    /**init mpi**/
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Get_processor_name(process_name,&name_len);

    total_start_time = MPI_Wtime();


    /**init excute parameters.**/
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


    /**read**/
    start_time = MPI_Wtime();
        my_read();
    end_time = MPI_Wtime();
    read_time = end_time-start_time;


    /**sort**/
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



    /**write**/
    start_time = MPI_Wtime();
        my_write();
    total_end_time = end_time = MPI_Wtime();
    write_time = end_time-start_time;

    total_time = total_end_time - total_start_time;
    compution_time = total_time - communication_time - read_time - write_time;

    if(PRINT_TIME)
    {
        printf("total_time: %f\n communication_time: %f\n compution_time: %f\n read_time: %f\n write_time: %f\n",
            total_time, communication_time, compution_time, read_time, write_time );
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
        /*1. compare and swap local data. */
        for(k=start;k<current_process_int_num-1;k+=2)
        {
            swaped = swaped | my_compare_swap(data,k,k+1);
        }

        /*2. determine if there is a need to send a element to the left and left process **/
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



        for(k=start?0:1;k<current_process_int_num-1;k+=2)
        {
            swaped = swaped | data[k]>data[k+1];
            if(swaped)
            {
                break;
            }
        }

        /*if there is a need to send a element to the left process, send and receive, then compare and keep the bigger one*/
        /*if there is a need to send a element to the right process, send and receive, then compare and keep the smaller one*/
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

        /**if current process is not controller: inform the 0th process whether any swap happened in previous steps,**/
        /**then according the received message to decide if or not continue the loop**/
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
        }
        /**if current process is controller: after receiving message from all others,determine if or not continue the loop and  inform others**/
        else
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

    /*keey the smaller one*/
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
    /*keep the bigger one*/
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

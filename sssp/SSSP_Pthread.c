#include "include.h"
#include<pthread.h>

pthread_t * thread_handles;
pthread_barrier_t barrier;
int *S ;
int *dist;
int *prev;
int **A ;

int *min_distance;
int *min_vertex;

void synchronize(double* barrier_time);

void dijkstra(void* args)
{
    struct timeval start,end;
    gettimeofday(&start, NULL );
    double thread_time = .0;
    double thread_synchronize_time = .0;
    double thread_computing_time = .0;

    int thread_rank = (long)args;
    int reminder,quotient;
    int current_process_start_index;
    int current_process_int_num;
    int current_process_end_index;
    reminder = vertexes_number%threads_number;
    quotient = vertexes_number/threads_number;
    if(thread_rank>=reminder)
    {
        current_process_start_index = quotient*thread_rank + reminder;
        current_process_int_num = quotient;
    }else
    {
        current_process_start_index = (quotient+1)*thread_rank;
        current_process_int_num = quotient+1;
    }
    current_process_end_index = current_process_start_index+current_process_int_num-1;

    for(int i=0; i<vertexes_number; i++)
    {
        min_distance[thread_rank] = INT_MAX;
        min_vertex[thread_rank] = source_vertex;                   // 找出当前未使用的点j的dist[j]最小值
        for(int j=current_process_start_index; j<=current_process_end_index; ++j)
            if((!S[j]) && dist[j]<min_distance[thread_rank])
            {
                min_vertex[thread_rank] = j;                             // u保存当前邻接点中距离最小的点的号码
                min_distance[thread_rank] = dist[j];
            }

        synchronize(&barrier_time);

        int u = source_vertex;
        int minist = INT_MAX;
        for(int j=0; j<threads_number; j++)
        {
            if(min_distance[j]<minist)
            {
                u = min_vertex[j];
                minist = min_distance[j];
            }
        }
        S[u] = 1;

        for(int j=current_process_start_index; j<=current_process_end_index; j++)
            if((!S[j]) && A[u][j]<INT_MAX)
            {
                if(dist[u] + A[u][j] < dist[j])     //在通过新加入的u点路径找到离source_vertex点更短的路径
                {
                    dist[j] = dist[u] + A[u][j];    //更新dist
                    prev[j] = u;                    //记录前驱顶点
                }
            }
        synchronize(&barrier_time);
    }

    gettimeofday(&end, NULL );
  	long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    thread_time = ((double)timeuse)/1000.0;
    thread_computing_time = thread_time - thread_synchronize_time;
    if(PRINT_TIME)
    {
      printf("thread %d\n thread_time %f\n thread_computing_time: %f\n thread_synchronize_time: %f\n",
              thread_rank, thread_time, thread_computing_time, thread_synchronize_time);
    }
}


void init_dijkstra()
{
    S = (int*)malloc(sizeof(int)*vertexes_number);
    dist = (int*)malloc(sizeof(int)*vertexes_number);
    thread_handles = malloc(threads_number * sizeof(pthread_t));
    min_distance = malloc(threads_number * sizeof(int));
    min_vertex = malloc(threads_number * sizeof(int));

    prev = result_collect;
    A = graph_weight;

    for(int i=0; i<vertexes_number; ++i)
    {
        dist[i] = A[source_vertex][i];
        S[i] = 0;
        if(dist[i] == INT_MAX)
            prev[i] = -1;
        else
            prev[i] = source_vertex;
    }

    dist[source_vertex] = 0;
    S[source_vertex] = 1;
}

void finalize_dijkstra()
{
    free(S);
    free(dist);
    free(thread_handles);
    free(min_distance);
    free(min_vertex);
}

void synchronize(double* barrier_time)
{
    struct timeval start,end;
    gettimeofday(&start, NULL );
    pthread_barrier_wait(&barrier);
    gettimeofday(&end, NULL );
  	long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    *barrier_time += ((double)timeuse)/1000.0;
}

void my_pthread_execute()
{
    long thread;

    init_dijkstra();

    pthread_barrier_init(&barrier,NULL, threads_number);
    for(thread=0 ; thread<threads_number; thread++)
        pthread_create(&thread_handles[thread], NULL, (void*)dijkstra, (void *)thread);

    for(thread=0; thread<threads_number; thread++)
        pthread_join(thread_handles[thread], NULL);

    finalize_dijkstra();
}


int main(int argc,char *argv[])
{
    struct timeval start,end;
    gettimeofday(&start, NULL );

    my_init(argc, argv);
    my_pthread_execute();
    print_result(result_collect);
    my_global_free();

    gettimeofday(&end, NULL );
  	long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    total_time = ((double)timeuse)/1000.0;
    if(PRINT_TIME)
    {
        printf("process : \n total_time: %f\n fileio_time: %f\n",
                total_time, fileio_time);
    }
    return 0;
}

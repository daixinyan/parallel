#include "include.h"
#include <pthread.h>

pthread_t * thread_handles;
pthread_barrier_t barrier;
int *S ;
int *dist;
int *prev;
int **A ;

int *min_distance;
int *min_vertex;

void dijkstra()
{
    
    for(int i=0; i<vertexes_number; i++)
    {


        for(int j=0; j<(threads_number); j++)
        {
          min_distance[j] = INT_MAX;
          min_vertex[j] = source_vertex;                   // 找出当前未使用的点j的dist[j]最小值
        }

        #pragma omp parallel for  schedule(dynamic,5)  num_threads(threads_number)
        for(int j=0; j<vertexes_number; ++j)
        {
            int thread_rank = omp_get_thread_num();
            if((!S[j]) && dist[j]<min_distance[thread_rank])
            {
                min_vertex[thread_rank] = j;                             // u保存当前邻接点中距离最小的点的号码
                min_distance[thread_rank] = dist[j];
            }
        }

        int u = source_vertex;
        int minist = INT_MAX;

        for(int j=0; j<(threads_number); j++)
        {
            if(min_distance[j]<minist)
            {
                u = min_vertex[j];
                minist = min_distance[j];
            }
        }
        S[u] = 1;

        #pragma omp parallel for  schedule(dynamic,5)  num_threads(threads_number)
        for(int j=0; j<vertexes_number; j++)
            if((!S[j]) && A[u][j]<INT_MAX)
            {
                if(dist[u] + A[u][j] < dist[j])     //在通过新加入的u点路径找到离source_vertex点更短的路径
                {
                    dist[j] = dist[u] + A[u][j];    //更新dist
                    prev[j] = u;                    //记录前驱顶点
                }
            }
    }


}


void init_dijkstra()
{
    S = (int*)malloc(sizeof(int)*vertexes_number);
    dist = (int*)malloc(sizeof(int)*vertexes_number);
    thread_handles = malloc((threads_number) * sizeof(pthread_t));
    min_distance = malloc((threads_number) * sizeof(int));
    min_vertex = malloc((threads_number) * sizeof(int));

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

void my_pthread_execute()
{
    init_dijkstra();
    dijkstra();
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
    total_time = ((double)timeuse)/1000000.0;
    if(PRINT_TIME)
    {
        printf("process : \n total_time: %f\n fileio_time: %f\n",
                total_time, fileio_time);
    }
    return 0;
}

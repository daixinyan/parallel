#include "include.h"


void dijkstra()
{
      int *S = (int*)malloc(sizeof(int)*vertexes_number);
      int *dist = (int*)malloc(sizeof(int)*vertexes_number);
      int *pre = result_collect;
      int **A = graph_weight;
      int n=vertexes_number;

      for(int i=0; i<n; ++i)
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
   　　
 　　 for(int i=1; i<n; i++)
 　　 {
       　　int mindist = INT_MAX;
       　　int u = source_vertex; 　　                  // 找出当前未使用的点j的dist[j]最小值
      　　 for(int j=0; j<n; ++j)
      　　    if((!S[j]) && dist[j]<mindist)
      　　    {
         　　       u = j;                             // u保存当前邻接点中距离最小的点的号码
         　 　      mindist = dist[j];
       　　   }
       　　S[u] = 1;
       　　for(int j=0; j<n; j++)
       　　    if((!S[j]) && A[u][j]<INT_MAX)
       　　    {
           　    　if(dist[u] + A[u][j] < dist[j])     //在通过新加入的u点路径找到离source_vertex点更短的路径
           　    　{
                   　　dist[j] = dist[u] + A[u][j];    //更新dist
                   　　prev[j] = u;                    //记录前驱顶点
            　　    }
        　    　}
   　　}
   free(S);
}


int main(int argc,char *argv[])
{
    my_init(argc, argv);
    dijkstra();
    print_result(result_collect);
    my_global_free();

    return 0;
}

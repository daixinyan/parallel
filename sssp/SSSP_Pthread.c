#include "include.h"

void floyd(){
    int i, k, j;
    for(i = 0;i < vertexes_number; i++)
    {
      result_collect[i] = source_vertex;
    }
    for(k = 0;k < vertexes_number;k++)
            for(j = 0;j < vertexes_number;j++){
                if( graph_weight[source_vertex][k] + graph_weight[k][j] < graph_weight[source_vertex][j])
                {
                    graph_weight[source_vertex][j] = graph_weight[source_vertex][k] + graph_weight[k][j];
                    result_collect[j] = k;
                }
            }
}


int main(int argc,char *argv[])
{
    my_init(argc, argv);
    floyd();
    print_result(result_collect);
    my_global_free();

    return 0;
}

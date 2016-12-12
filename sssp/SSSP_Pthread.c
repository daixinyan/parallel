#include "include.h"


int main(int argc,char *argv[])
{
    my_init(argc, argv);
    my_global_free();
    floyd();
    print_result(result_collect);
    return 0;
}

void floyd(){
    for(int k = 0;k < vertexes_number;k++)
            for(int j = 0;j < vertexes_number;j++){
                if( graph_weight[source_vertex][k] + graph_weight[k][j] < graph_weight[source_vertex][j])
                {
                    graph_weight[source_vertex][j] = graph_weight[source_vertex][k] + graph_weight[k][j];
                    result_collect[j] = k;
                }
            }
}

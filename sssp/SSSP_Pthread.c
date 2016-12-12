#include "include.h"

void floyd(){
    int k, j;
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
    // floyd();
    my_global_free();
    print_result(result_collect);
    return 0;
}

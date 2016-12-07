#include "include.h"


int main(int argc,char *argv[])
{
    my_init(argc, argv);
    my_mpi_finalize();
    return 0;
}

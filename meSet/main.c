#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{

    int i, j;
	#pragma omp parallel for private(i,j)
	for(i=0; i<10; i++)
	{
	    for(j=0; j<10; j++)
	    {
	        #pragma omp critical
            {
                printf("%d, %d %d\n", i,j, omp_get_thread_num());
            }
	    }
	}
    return 0;
}

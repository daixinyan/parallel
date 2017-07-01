#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "prime.h"
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

using namespace std;

//static int threadNum = 1;
static long MAX_NUM = 200 * 1000  ;


int main(int argc, char ** args) {


    int i;
    int count = 0;
    struct timeval start,end;
    gettimeofday(&start, NULL );

    #pragma omp parallel for  private(i) schedule(dynamic,20)  num_threads(3) reduction(+:count)
    for(long i=3; i<=MAX_NUM; i += 2 ) {
        if(isPrime(i))
	    count++;
    }
    
    cout<<count<<endl;
    gettimeofday(&end, NULL);

    long timeUse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    cout << timeUse << endl;
    return 0;
}

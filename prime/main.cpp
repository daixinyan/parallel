#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "prime.h"
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

using namespace std;

//static int threadNum = 1;
static long MAX_NUM = 10 * 1000  ;


int main(int argc, char ** args) {


    int i;
    #pragma omp parallel for  private(i) schedule(static,1)  num_threads(parameters.number_of_threads)
    for(long i=3; i<=MAX_NUM; i += 2 ) {
        isPrime(i);
    }
    struct timeval start,end;
    gettimeofday(&start, NULL );


    gettimeofday(&end, NULL);

    long timeUse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    cout << timeUse << endl;
    return 0;
}
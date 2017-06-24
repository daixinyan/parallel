#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "prime.h"
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

using namespace std;

static int threadNum = 1;
static long MAX_NUM = 1000 * 1000  ;


void pthread_execute(void* p) {
    int num = threadNum;
    int threadIndex = (long)p;

    for(long i=threadIndex*2+3; i<=MAX_NUM; i += num*2 ) {
        isPrime(i);
    }
}

int main(int argc, char ** args) {

    if(argc>1) {
        threadNum = atoi(args[1]);
    }
    if(argc>2) {
        MAX_NUM = atol(args[2]);
    }

    struct timeval start,end;
    gettimeofday(&start, NULL );

    pthread_t* pthread_handles = new pthread_t[threadNum];


    for (long thread = 0; thread < threadNum ; ++thread) {
        pthread_create(&pthread_handles[thread], NULL,(void* (*)(void*))pthread_execute, (void*)thread);
    }
    for(long thread=0; thread<threadNum; thread++)
        pthread_join(pthread_handles[thread], NULL);

    gettimeofday(&end, NULL);

    long timeUse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    cout << timeUse << endl;
    return 0;
}
#include <iostream>
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread(
#include <stdio.h>
#include "prime.h"
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

using namespace std;

static int threadNum = 6;
static const long MAX_NUM = 50 * 1000  ;

static pthread_barrier_t barrier;

void pthread_execute(void* p) {
    int num = threadNum;
    int threadIndex = (int)p;

    for(long i=threadIndex*2+3; i<=MAX_NUM; i += num*2 ) {
        isPrime(i);
    }

    pthread_barrier_wait(&barrier);
}

int main() {

    struct timeval start,end;
    gettimeofday(&start, NULL );

    pthread_t* pthread_handles = new pthread_t[threadNum];

    pthread_barrier_init(&barrier, NULL, threadNum);

    for (long thread = 0; thread < threadNum ; ++thread) {
        pthread_create(&pthread_handles[thread], NULL, pthread_execute, (void*)thread);
    }

    gettimeofday(&end, NULL);

    long timeUse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    cout << timeUse << endl;
    return 0;
}
#include <iostream>
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread(
#include <stdio.h>
#include "prime.h"
#include <time.h>

using namespace std;

static int threadNum = 6;
static const long MAX_NUM = 500 * 1000  ;

static inline unsigned int __stdcall threadFunction(void *p) {

    int num = threadNum;
    int threadIndex = (int)p;

    for(long i=threadIndex*2+3; i<=MAX_NUM; i += num*2 ) {
        isPrime(i);
    }
}


int execute() {
    HANDLE* handle = new HANDLE[threadNum];
    clock_t  start, end;
    start = clock();
    for (int i = 0; i < threadNum; ++i) {
        handle[i] = (HANDLE)_beginthreadex(NULL, 0, thread, (void*)i, 0, NULL);
    }

    WaitForMultipleObjects(threadNum, handle, TRUE, INFINITE);

    end = clock();
    delete handle;

    cout<<((double)(end-start)/CLOCKS_PER_SEC)<<endl;
    return 0;
}
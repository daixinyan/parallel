//
// Created by darxan on 2017/6/17.
//

#ifndef PRIME_PRIME_H
#define PRIME_PRIME_H



inline bool isPrime(long number) {
    for (int i=3; i<number/2+1; i++) {
        if(number%i==0) {
            return false;
        }
    }
    return true;
}
#endif //PRIME_PRIME_H

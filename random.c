#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ULONG 0xffffffffffffffff
#define MAX_UINT 0xffffffff

extern pthread_key_t seed_key;

void initRandom()
{
    unsigned long seed_;
    struct timeval tv;

    pthread_key_create(&seed_key, NULL);
    gettimeofday(&tv, NULL);
    seed_ = tv.tv_usec & MAX_ULONG;
    pthread_setspecific(seed_key, (void *)seed_);
}

unsigned int getRandom(unsigned int s)
{
    unsigned long x0;

    x0 = (unsigned long)pthread_getspecific(seed_key);
    x0 = (x0 * 1103515245 + 12345) & 0x7fffffff;
    pthread_setspecific(&seed_key, (void *)x0);

    return ((x0 & MAX_UINT) * s) >> 32;
}

int getRand()
{
    return rand();
}
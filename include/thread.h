#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIM_TIME 1

void thread_init(boardInfo *bi);
void thread_start(boardInfo *bi);
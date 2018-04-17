#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define BOARD_ROW 19
#define BOARD_COLUMN 19
#define STATUS 3
#define SIM_TIME 1
#define BLACK 1
#define WHITE 2
#define FIRST 1
#define SECOND 2
//通过CPU的核心数量来确定线程的数量
#define THREAD_NUMBER (sysconf(_SC_NPROCESSORS_CONF) * 2)

static pthread_key_t seed_key;
unsigned int board_status[BOARD_ROW * BOARD_COLUMN][STATUS];

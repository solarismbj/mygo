#include "mygo_main.h"

extern void initRandom();
extern unsigned int getRandom(unsigned int s);

int main(int argc, char **argv)
{
    initRandom();
    srand((int)time(0));
    for(int i = 0; i < 10; i++)
    {
        //unsigned int seed = getRandom(0xffffffff);
        //printf("%d\n", getRandom(seed + i));
        printf("%d\n", getRand());
    }
    return 0;
}
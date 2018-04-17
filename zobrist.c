#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mygo.h"

#define ROW 19
#define COLUMN 19
#define STATUS 3

extern unsigned int board_status[ROW * COLUMN][STATUS];

void zobrist_create()
{
  srand((unsigned int)time(0));

  for(int i = 0; i < ROW * COLUMN; i++)
  {
    for(int j = 0; j < STATUS; j++)
    {
      board_status[i][j] = (unsigned int)rand();
    }
  }
}

unsigned int zobrist_generate_root()
{
  unsigned int z = 0;

  for(int i = 0; i < ROW * COLUMN; i++)
  {
    z ^= board_status[i][0];
  }

  return z;
}

void zobrist_update(unsigned int *zobrist_value, int pos, int old_status, int status)
{
  *zobrist_value ^= board_status[pos][old_status];
  *zobrist_value ^= board_status[pos][status];
}

#include <pthread.h>
#include <string.h>

#include "uct.h"

#define BOARD_ROW 19
#define BOARD_COLUMN 19
#define MAX_ULONG 0xffffffffffffffff
#define MAX_UINT 0xffffffff
#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define YES 1
#define NO -1
#define BLOCK_END -1
#define STATUS 3

int boardcount = 0;
const int direction[4][2] = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

typedef struct boardInfo_
{
    pthread_t tid;
    int id;
    int board[BOARD_ROW][BOARD_COLUMN];
    int bc;
    int board_black_emptypos;
    int board_white_emptypos;
    int moveflag;
    int group_id;
    unsigned int seed;
    //棋盘上黑棋的空位置信息
    int board_black_empties[BOARD_ROW * BOARD_COLUMN];
    //棋盘上白棋的空位置信息
    int board_white_empties[BOARD_ROW * BOARD_COLUMN];
    //棋盘上气的位图
    unsigned int liberty_map[BOARD_ROW * BOARD_COLUMN * BOARD_ROW];
    int block[BOARD_ROW * BOARD_COLUMN];  //棋子的位置和其所属棋块的标识信息
    int block_list[BOARD_ROW * BOARD_COLUMN];  //棋块链表
    int block_map[BOARD_ROW * BOARD_COLUMN];  //棋块位图
    int block_end[BOARD_ROW * BOARD_COLUMN];  //棋块的末端位置
    int block_liberty[BOARD_ROW * BOARD_COLUMN];  //棋块的气
    int black_stone;
    int white_stone;
    int black_dead;
    int white_dead;
    int result_success[BOARD_ROW * BOARD_COLUMN];
    int result_failure[BOARD_ROW * BOARD_COLUMN];
    int step;
    double s_time;
    uct_t *rootnode;
}boardInfo;

void initBoard();
void restoreBoard(boardInfo *b);
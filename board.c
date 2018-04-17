#include "board.h"

void initBoard()
{
    zobrist_create();
}

void restoreBoard(boardInfo *b)
{
    b->group_id = 1;
    b->moveflag = 1;
    b->board_black_emptypos = BOARD_ROW * BOARD_COLUMN;
    b->board_white_emptypos = b->board_black_emptypos;
    b->black_stone = 0;
    b->white_stone = 0;
    b->black_dead = 0;
    b->white_dead = 0;
    memset(b->board, EMPTY, sizeof(b->board));
    memcpy(b->board_black_empties, board_status, sizeof(board_status));
    memcpy(b->board_white_empties, board_status, sizeof(board_status));
    memset(b->liberty_map, 0, sizeof(b->liberty_map));
    memset(b->block, 0, sizeof(b->block));
    memset(b->block_list, 0, sizeof(b->block_list));
    memset(b->block_map, 0, sizeof(b->block_map));
    memset(b->block_end, 0, sizeof(b->block_end));
    memset(b->block_liberty, 0, sizeof(b->block_liberty));
    rootnode = null;
}
#include "thread.h"
#include "board.h"
#include "uct.h"

uct_t *root_node;

double gettime()
{
    struct timeval tv;
    double ti;

    gettimeofday(&tv, NULL);
    ti = (double)(tv.tv_sec + tv.tv_usec / 1000000.0);
    return ti;
}

static void *thread_function(void *arg)
{
    unsigned int i;
    double start_time;
    
    int step = 0;
    int emptypos, j;
    int first_step;
    double s_time = SIM_TIME;
    boardInfo *bi;

    bi = (boardInfo *)arg;
    bi->rootnode = root_node;
    time_used = 0.0;
    start_time = gettime();
    printf("线程%d在%lf开始\n", bi->id, start_time);

    while(time_used <= s_time)
    {
        uct_search(bi, s_time);
        //print_block_map(bi);
        //printf("第%d手：i=%d\n", ++step, i);
        if(bi->moveflag == 1)
        {
            if(bi->board_black_emptypos == 0)
            {
                bi->moveflag = 0;
            }
            else
            {
                if(bi->board_black_emptypos > 1)
                {
                    i = uct_search(bi);
                    //生成0至360的整数
                    //i = getRandom(bi->seed) % bi->board_black_emptypos;
                }
                else
                {
                    if(bi->board_black_emptypos == 1)
                    {
                        i = bi->board_black_emptypos - 1;
                    }
                }

                if((check_block(bi, bi->board_black_empties[i], BLACK)) == YES)
                {
                    //printf("R=%d,C=%d\n", bi->board_empties[i] / BOARD_ROW, bi->board_empties[i] % BOARD_COLUMN);
                    bi->board[bi->board_black_empties[i] / BOARD_ROW][bi->board_black_empties[i] % BOARD_COLUMN] = BLACK;
                    bi->black_stone++;
                    //printf("liberty[%d](%d)=%d\n", bi->block_map[bi->board_black_empties[i]], bi->board_black_empties[i], bi->block_liberty[bi->block_map[bi->board_black_empties[i]]]);
                    if(step == 0)
                    {
                        first_step = bi->board_black_empties[i];
                    }
                    //sgf[++step] = ';';
                    //sgf[++step] = 'B';
                    //sgf[++step] = '[';
                    //sgf[++step] = 97 + bi->board_black_empties[i] % BOARD_COLUMN;
                    //sgf[++step] = 97 + bi->board_black_empties[i] / BOARD_ROW;
                    //sgf[++step] = ']';
                    emptypos = bi->board_black_empties[i];
                    bi->board_black_empties[i] = bi->board_black_empties[bi->board_black_emptypos - 1];
                    bi->board_black_emptypos--;
                    for(j = 0; j < bi->board_white_emptypos; j++)
                    {
                        if(emptypos == bi->board_white_empties[j])
                        {
                            bi->board_white_empties[j] = bi->board_white_empties[bi->board_white_emptypos - 1];
                            bi->board_white_emptypos--;
                            break;
                        }
                    }
                    bi->moveflag = 0;
                }
                else
                {
                    bi->board_black_empties[i] = bi->board_black_empties[bi->board_black_emptypos - 1];
                    bi->board_black_emptypos--;
                }
            }
        }
        else
        {
            if(bi->board_white_emptypos == 0)
            {
                bi->moveflag = 1;
            }
            else
            {
                if(bi->board_white_emptypos > 1)
                {
                    //生成0至360的整数
                    i = getRandom(bi->seed) % bi->board_white_emptypos;
                }
                else
                {
                    if(bi->board_white_emptypos == 1)
                    {
                        i = bi->board_white_emptypos - 1;
                    }
                }

                if((check_block(bi, bi->board_white_empties[i], WHITE)) == YES)
                {
                    bi->board[bi->board_white_empties[i] / BOARD_ROW][bi->board_white_empties[i] % BOARD_COLUMN] = WHITE;
                    bi->white_stone++;
                    //printf("liberty[%d](%d)=%d\n", bi->block_map[bi->board_white_empties[i]], bi->board_white_empties[i], bi->block_liberty[bi->block_map[bi->board_white_empties[i]]]);
                    //sgf[++step] = ';';
                    //sgf[++step] = 'W';
                    //sgf[++step] = '[';
                    //sgf[++step] = 'a' + bi->board_white_empties[i] % BOARD_COLUMN;
                    //sgf[++step] = 'a' + bi->board_white_empties[i] / BOARD_ROW;
                    //sgf[++step] = ']';
                    emptypos = bi->board_white_empties[i];
                    bi->board_white_empties[i] = bi->board_white_empties[bi->board_white_emptypos - 1];
                    bi->board_white_emptypos--;
                    for(j = 0; j < bi->board_black_emptypos; j++)
                    {
                        if(emptypos == bi->board_black_empties[j])
                        {
                            bi->board_black_empties[j] = bi->board_black_empties[bi->board_black_emptypos - 1];
                            bi->board_black_emptypos--;
                            break;
                        }
                    }
                    bi->moveflag = 1;
                }
                else
                {
                    bi->board_white_empties[i] = bi->board_white_empties[bi->board_white_emptypos - 1];
                    bi->board_white_emptypos--;
                }
            }
        }
        //printf("emptypos=%d\n", bi->emptypos);
        //print_block_map(bi);
        //print_board(bi);
        time_used = gettime() - start_time;
        if(bi->board_black_emptypos == 0 && bi->board_white_emptypos == 0)
        {
            //print_block_map(bi);
            //print_liberty(bi);
            int bd = 0;
            int wd = 0;
            for(i = 0; i < BOARD_ROW * BOARD_COLUMN; i++)
            {
                if(bi->block_liberty[bi->block_map[i]] == 1)
                {
                    if(bi->board[i / BOARD_ROW][i % BOARD_COLUMN] == BLACK)
                    {
                        bd++;
                    }
                    else
                    {
                        wd++;
                    }
                }
            }
            if((bi->black_stone + bi->white_dead + wd - bi->black_dead - bd - 180.5 - 3.75) > 0.0)
            {
                bi->result_success[first_step]++;
            }
            else
            {
                bi->result_failure[first_step]++;
            }
            //print_board(bi);
            bi->bc++;
            restoreBoard(bi);
            //sgf[step + 6] = ')';
            //sgf[step + 7] = '\0';
            //write_to_sgf_file(bi, sgf, bi->bc);
            step = 0;
        }
    }
    printf("线程%d在%lf结束\n", bi->id, gettime());
    printf("线程%d用时是%f秒。\n", bi->id, time_used);
    printf("线程%d一共模拟了%d局。\n", bi->id, bi->bc);
    //free(bi);
    return NULL;
}

void thread_init(boardInfo *bi)
{
    int res;

    res = pthread_create(&bi->tid, NULL, thread_function, bi);
    if(res != 0)
    {
        perror("The thread creation failed!");
        exit(EXIT_FAILURE);
    }
}

void thread_start(boardInfo *bi, uct_t *root)
{
    root_node = root;
    pthread_join(bi->tid, NULL);
}
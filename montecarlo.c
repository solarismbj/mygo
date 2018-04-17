#include "montecarlo.h"

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
    pthread_setspecific(seed_key, (void *)x0);
    return ((x0 & MAX_UINT) * s) >> 32;
}

void initBoard()
{
    int i;

    for(i = 0; i < BOARD_ROW * BOARD_COLUMN; i++)
    {
        board_status[i] = i;
    }
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
}

double gettime()
{
    struct timeval tv;
    double ti;

    gettimeofday(&tv, NULL);
    ti = (double)(tv.tv_sec + tv.tv_usec / 1000000.0);
    return ti;
}

//建立新的棋块
void new_block(boardInfo *b, int p)
{
    //printf("block_map[%d]=%d\n", p, b->block_map[p]);
    b->block[b->group_id] = p;
    b->block_list[p] = -1;
    b->block_map[p] = b->group_id;
    b->block_end[b->group_id] = p;
    b->group_id++;
    //printf("新建棋块成功。\n");
}

//加入棋块
void addto_block(boardInfo *b, int p, int g)
{
    b->block_map[p] = g;
    b->block_list[b->block_end[g]] = p;
    b->block_list[p] = -1;
    b->block_end[g] = p;
    b->block_liberty[g]--;
    b->liberty_map[(g - 1) * BOARD_ROW + p / BOARD_ROW] = b->liberty_map[(g - 1) * BOARD_ROW + p / BOARD_ROW] & ((0x40000 >> (p % BOARD_COLUMN)) ^ 0x7ffff);
}

//合并棋块
void merge_block(boardInfo *b, int g1, int g2, int p)
{
    int pos;

    pos = b->block[g2];
    b->block[g2] = 0;
    b->block_list[b->block_end[g1]] = pos;
    while(pos != BLOCK_END)
    {
        b->block_map[pos] = g1;
        pos = b->block_list[pos];
    }
    b->block_end[g1] = b->block_end[g2];
    b->block_end[g2] = 0;
    b->liberty_map[(g2 - 1) * BOARD_ROW + p / BOARD_ROW] = b->liberty_map[(g2 - 1) * BOARD_ROW + p / BOARD_ROW] & ((0x40000 >> (p % BOARD_COLUMN)) ^ 0x7ffff);
    for(int i = 0; i < BOARD_ROW; i++)
    {
        b->liberty_map[(g1 - 1) * BOARD_ROW + i] = b->liberty_map[(g1 - 1) * BOARD_ROW + i] | b->liberty_map[(g2 - 1) * BOARD_ROW + i];
        b->liberty_map[(g2 - 1) * BOARD_ROW + i] = b->liberty_map[(g2 - 1) * BOARD_ROW + i] & 0x0;
    }
}

//提子函数
void remove_block(boardInfo *b, int p, int color)
{
    int gid;
    int pos, pos_t;
    int newx, newy, newp;
    int x, y;

    gid = b->block_map[p];
    pos = b->block[gid];
    while(pos != BLOCK_END)
    {
        x = pos % BOARD_COLUMN;
        y = pos / BOARD_ROW;
        //printf("remove_pos=%d\n", pos);
        for(int i = 0; i < 4; i++)
        {
            newx = pos % BOARD_COLUMN + direction[i][0];
            newy = pos / BOARD_ROW + direction[i][1];
            newp = newy * BOARD_ROW + newx;
            if(newx > -1 && newx < 19 && newy > -1 && newy < 19)
            {
                if(b->board[newy][newx] == color)
                {
                    if((b->liberty_map[(newp - 1) * BOARD_ROW + y] & (0x40000 >> x)) == 0)
                    {
                        b->liberty_map[(newp - 1) * BOARD_ROW + y] = b->liberty_map[(newp - 1) * BOARD_ROW + y] | (0x40000 >> x);
                        b->block_liberty[b->block_map[newp]]++;
                    }
                }
            }
        }
        b->board[pos / BOARD_ROW][pos % BOARD_COLUMN] = 0;
        if(color == BLACK)
        {
            b->white_dead++;
        }
        else
        {
            b->black_dead++;
        }
        pos_t = pos;
        pos = b->block_list[pos_t];
        b->block_list[pos_t] = 0;
        b->block_map[pos_t] = 0;
    }
    b->block[gid] = 0;
    b->block_end[gid] = 0;
    b->block_liberty[gid] = 0;
}

//检查落子之后的棋盘状态
int check_block(boardInfo *b, int p, int c)
{
    int x, y;
    int newx, newy;
    int i;
    void *block_temp;
    void *liberty_map_temp;
    int newg;
    int newp;
    unsigned int liberty_map[19];

    //printf("开始执行check_block函数。\n");
    block_temp = malloc(5 * sizeof(b->block));
    memcpy(block_temp, b->block, sizeof(b->block));
    memcpy(block_temp + sizeof(b->block), b->block_list, sizeof(b->block_list));
    memcpy(block_temp + 2 * sizeof(b->block), b->block_map, sizeof(b->block_map));
    memcpy(block_temp + 3 * sizeof(b->block), b->block_end, sizeof(b->block_end));
    memcpy(block_temp + 4 * sizeof(b->block), b->block_liberty, sizeof(b->block_liberty));
    memset(liberty_map, 0, sizeof(liberty_map));
    liberty_map_temp = malloc(sizeof(b->liberty_map));
    memcpy(liberty_map_temp, b->liberty_map, sizeof(b->liberty_map));
    x = p % BOARD_COLUMN;
    y = p / BOARD_ROW;
    //printf("x=%d,y=%d\n", x, y);
    for(i = 0; i < 4; i++)
    {
        newx = x + direction[i][0];
        newy = y + direction[i][1];
        newp = newy * BOARD_ROW + newx;
        newg = b->block_map[newp];
        if(newx > -1 && newx < 19 && newy > -1 && newy < 19)
        {
            if(newg > 0 && (b->liberty_map[(newg - 1) * BOARD_ROW + y] & (0x40000 >> x)) > 0)
            {
                b->liberty_map[(newg - 1) * BOARD_ROW + y] = b->liberty_map[(newg - 1) * BOARD_ROW + y] & ((0x40000 >> x) ^ 0x7ffff);
                b->block_liberty[newg]--;
            }

            //printf("newx=%d,newy=%d\n", newx, newy);
            if(b->board[newy][newx] != EMPTY)
            {
                if(b->board[newy][newx] == c)
                {
                    if(b->block_map[p] == 0)
                    {
                        addto_block(b, p, newg);
                    }
                    else
                    {
                        if(b->block_map[p] != b->block_map[newp])
                        {
                            merge_block(b, b->block_map[p], newg, p);
                        }
                    }
                }
                else
                {
                    if(b->block_map[p] == 0)
                    {
                        new_block(b, p);
                    }
                    if(b->block_liberty[b->block_map[newp]] == 0)
                    {
                        //printf("block=%d(%d)产生提子\n", b->block_map[p], p);
                        //printf("被提子的气数=%d\n", b->block_liberty[b->block_map[newp]]);
                        remove_block(b, newp, c);
                        liberty_map[newy] = 0x40000 >> newx;
                    }
                }
            }
            else
            {
                if(b->block_map[p] == 0)
                {
                    //printf("建立新棋块\n");
                    //printf("id=%d,block_map[%d]=%d\n", b->id, p, b->block_map[p]);
                    new_block(b, p);
                    b->liberty_map[(b->block_map[p] - 1) * BOARD_ROW] = b->liberty_map[(b->block_map[p] - 1) * BOARD_ROW] | (0x40000 >> x);
                    b->block_liberty[b->block_map[p]]++;
                }
                else
                {
                    if((b->liberty_map[b->block_map[p]] & (0x40000 >> x)) == 0)
                    {
                        b->liberty_map[(b->block_map[p] - 1) * BOARD_ROW] = b->liberty_map[(b->block_map[p] - 1) * BOARD_ROW] | (0x40000 >> x);
                        b->block_liberty[b->block_map[p]]++;
                    }
                }
            }
        }
    }

    //由于形成“自杀”状态，所以是无效行棋，必须恢复初始状态
    if(b->block_liberty[b->block_map[p]] == 0)
    {
        memcpy(b->block, block_temp, sizeof(b->block));
        memcpy(b->block_list, block_temp + sizeof(b->block), sizeof(b->block));
        memcpy(b->block_map, block_temp + 2 * sizeof(b->block), sizeof(b->block));
        memcpy(b->block_end, block_temp + 3 * sizeof(b->block), sizeof(b->block));
        memcpy(b->block_liberty, block_temp + 4 * sizeof(b->block), sizeof(b->block));
        memcpy(b->liberty_map, liberty_map_temp, sizeof(b->liberty_map));
        free(block_temp);
        free(liberty_map_temp);
        return NO;
    }
    free(block_temp);
    free(liberty_map_temp);
    return YES;
}

void print_block_map(boardInfo *b)
{
    int i, j;

    for(i = 0; i < BOARD_ROW; i++)
    {
        for(j = 0; j < BOARD_COLUMN; j++)
        {
            printf("%d ", b->block_map[i * BOARD_ROW + j]);
        }
        printf("\n");
    }
    printf("\n");
}


void print_board(boardInfo *b)
{
    for(int i = 0; i < BOARD_ROW; i++)
    {
        for(int j = 0; j < BOARD_COLUMN; j++)
        {
            printf("%d ", b->board[i][j]);
        }
        printf("\n");
    }
    printf("BLACK=%d,WHITE_DEAD=%d,WHITE=%d,BALCK_DEAD=%d\n", b->black_stone, b->white_dead, b->white_stone, b->black_dead);
    int bd = 0;
    int wd = 0;
    for(int i = 0; i < BOARD_ROW * BOARD_COLUMN; i++)
    {
        if(b->block_liberty[b->block_map[i]] == 1)
        {
            if(b->board[i / BOARD_ROW][i % BOARD_COLUMN] == BLACK)
            {
                bd++;
            }
            else
            {
                wd++;
            }
        }
    }
    printf("BALCK(未提)=%d,WHITE(未提)=%d\n", bd, wd);
    printf("The result=%f\n", b->black_stone - b->black_dead - bd + b->white_dead + wd - 180.5 - 3.75);
    printf("\n");
}

void print_liberty(boardInfo *b)
{
    for(int i = 0; i < BOARD_ROW * BOARD_COLUMN; i++)
    {
        printf("block(%d)liberty=%d;", i, b->block_liberty[i]);
    }
    printf("\n");
}

void write_to_sgf_array(char *s, int step, char c, int p)
{
    int x, y;

    x = p % BOARD_COLUMN;
    y = p / BOARD_ROW;
    s[step] = ';';
    s[step + 1] = c;
    s[step + 2] = '[';
    s[step + 3] = 97 + x;
    s[step + 4] = 97 + y;
    s[step + 5] = ']';
}

void write_to_sgf_file(boardInfo *b, char *s, int bid)
{
    FILE *fp;
    char f1[10];
    char f2[10];
    char f[40] = "./sgf/sgf_";

    sprintf(f1, "%d", b->id);
    sprintf(f2, "%d", bid);
    strcat(f, f1);
    strcat(f, "_");
    strcat(f, f2);
    strcat(f, ".sgf");
    if((fp = fopen(f, "w")) == NULL)
    {
        printf("创建SGF文件时，出现错误，程序已经退出。\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%s", s);
    fclose(fp);
}

void print_result(int *s, int *f, int count)
{
    FILE *fp;

    if((fp = fopen("sim_result.txt", "w")) == NULL)
    {
    }
    for(int i = 0; i < BOARD_ROW * BOARD_COLUMN; i++)
    {
        fprintf(fp, "%d: Success-%f, Failure-%f\n", i, (float)s[i] / (float)count, (float)f[i] / (float)count);
    }
    fclose(fp);
}

static void *thread_function(void *arg)
{
    unsigned int i;
    double time_used;
    double start_time;
    boardInfo *bi;
    int step = 0;
    //char sgf[2170];
    int emptypos, j;
    int first_step;

    bi = (boardInfo *)arg;
    time_used = 0.0;
    start_time = gettime();
    printf("线程%d在%lf开始\n", bi->id, start_time);
    //print_go(bi);
    //sgf[0] = '(';
    while(time_used <= s_time)
    {
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
                    //生成0至360的整数
                    i = getRandom(bi->seed) % bi->board_black_emptypos;
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

int main(int argc, char *argv[])
{
    boardInfo *bi[THREAD_NUMBER];
    int result_s[BOARD_ROW * BOARD_COLUMN];
    int result_f[BOARD_ROW * BOARD_COLUMN];

    initBoard();
    initRandom();
    if(argc == 3)
    {
        s_time = atof(argv[1]);
        t_num = atoi(argv[2]);
    }
    else
    {
        s_time = SIM_TIME;
        t_num = THREAD_NUMBER;
    }
    int i;
    int res;
    double start_time, end_time;

    printf("PC的CPU数量是：%ld\n", sysconf(_SC_NPROCESSORS_CONF));
    srand((int)time(0));
    boardcount = 0;
    start_time = gettime();
    printf("主程序在%lf秒，开始创建线程。\n", start_time);
    for(i = 0; i < t_num; i++)
    {
        bi[i] = (boardInfo *)malloc(sizeof(boardInfo));
        bi[i]->id = i;
        bi[i]->seed = getRandom(MAX_UINT) + i;
        memset(bi[i]->result_success, 0, sizeof(bi[i]->result_success));
        memset(bi[i]->result_failure, 0, sizeof(bi[i]->result_failure));
        restoreBoard(bi[i]);
        res = pthread_create(&bi[i]->tid, NULL, thread_function, bi[i]);
        if(res != 0)
        {
            perror("The thread creation failed!");
            exit(EXIT_FAILURE);
        }
    }
    for(i = 0; i < t_num; i++)
    {
        pthread_join(bi[i]->tid, NULL);
    }
    end_time = gettime();
    printf("主程序在%lf秒结束，主程序最终用时为%lf秒。\n", end_time, end_time - start_time);
    memset(result_s, 0, sizeof(result_s));
    memset(result_f, 0, sizeof(result_f));
    for(i = 0; i < t_num; i++)
    {
        boardcount += bi[i]->bc;
        for(int j = 0; j < BOARD_ROW * BOARD_COLUMN; j++)
        {
            result_s[j] += bi[i]->result_success[j];
            result_f[j] += bi[i]->result_failure[j];
        }
        free(bi[i]);
    }
    print_result(result_s, result_f, boardcount);
    printf("一共模拟了%d局。\n", boardcount);
    printf("主程序用时是：%lf\n", gettime() - start_time);
    return 0;
}

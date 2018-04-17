#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "board.h"
#include "zobrist.h"

#define ROWS 19
#define COLUMNS 19
#define ITERATIONS 80000
#define LEVELS 5

typedef struct uct_t_
{
    int unexplored[ROWS * COLUMNS];
    unsigned int unexplored_length;
    int explored[ROWS * COLUMNS];
    int explored_length;
    struct uct_t_ *parent;
    struct uct_t_ **child;
    struct uct_t_ **bestchild;
    float win_rate;
    unsigned int wins;
    unsigned int loss;
    unsigned int visit_counts;
    unsigned int zobrist;
    int pos;
}uct_t;

void init_rootnode(uct_t *rootnode)
{
    for(int i = 0; i < ROWS * COLUMNS; i++)
    {
        rootnode->unexplored[i] = i;
        rootnode->explored[i] = 0;
    }
    rootnode->explored_length = 0;
    rootnode->bestchild = NULL;
    rootnode->child = (uct_t **)malloc(sizeof(uct_t));
    rootnode->parent = NULL;
    rootnode->loss = 0;
    rootnode->wins = 0;
    rootnode->win_rate = 0.0;
    rootnode->visit_counts = 0;
    rootnode->pos = -1;
    rootnode->unexplored_length = ROWS * COLUMNS;
    rootnode->zobrist = zobrist_generate_root();
}

/*
int get_rand(unsigned int length)
{
    srand((int)time(0));

    return rand() % length;
}

uct_t *bestchild(uct_t *parentnode)
{
    int pos = 0;
    float max_q_u = 0;

    for(int i = 0; i < sizeof(parentnode->child) / sizeof(uct_t); i++)
    {
        float q_u = parentnode->child[i]->win_rate + sqrt(log(parentnode->visit_counts) / 5 / parentnode->child[i]->visit_counts);
        if(q_u > max_q_u)
        {
            max_q_u = q_u;
            pos = i;
        }
    }

    return parentnode->child[pos];
}

*/

uct_t *node_create(int pos)
{
    uct_t *node = (uct_t *)malloc(sizeof(uct_t));

    node->pos = pos;

    return node;
}


uct_t *uct_select(uct_t *rootnode)
{
    uct_t *child;

    if(rootnode->unexplored_length > 0)
    {
        int unexploredpos = get_rand(rootnode->unexplored_length);
        int pos = unexploredpos;
        rootnode->unexplored[pos] = rootnode->unexplored[rootnode->unexplored_length];
        rootnode->unexplored_length--;
        rootnode->explored_length++;
        child = (uct_t *)malloc(sizeof(uct_t));
        child->pos = pos;
        memcpy(child->unexplored, rootnode->unexplored, rootnode->unexplored_length);
        rootnode->child = (uct_t **)realloc(rootnode->child, sizeof(uct_t) * rootnode->explored);
        rootnode->child[rootnode->explored_length - 1] = &child;
        child->bestchild = NULL;
        child->child = NULL;
        child->parent = rootnode;
        child->loss = 0;
        child->wins = 0;
        child->win_rate = 0.0;
        child->visit_counts = 0;
        child->unexplored_length = rootnode->unexplored_length;
    }
    else
    {
        return bestchild(rootnode);
    }

    return child;
}

uct_t *uct_expand(uct_t *childnode)
{
    uct_t *node;

    for(int i = 0; i < LEVELS; i++)
    {
        node = uct_select(childnode);
        childnode = node;
    }

    return node;
}

int uct_simulate(uct_t *node)
{
    int win = 1;
    int loss = -1;

    if()
    {
        return win;
    }

    return loss;
}

void uct_backup(uct_t *childnode)
{
}

int uct_search(boardInfo *bi)
{
    uct_t *child;
    double time_used = 0.0;
    double start_time;

    start_time = gettime();

    while(time_used <= bi->s_time)
    {
        child = uct_select(bi->rootnode);
        uct_expand(child);
        uct_simulate(child);
        uct_backup(child);
        time_used = gettime() - start_time;
    }

    bi->rootnode = child;

    return child->pos;
}

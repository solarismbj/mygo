#include "mygo.h"
#include "random.h"
#include "board.h"
#include "thread.h"
#include "uct.h"

int main(int argc, char *argv[])
{
  int result_s[BOARD_ROW * BOARD_COLUMN];
  int result_f[BOARD_ROW * BOARD_COLUMN];
  double s_time;  //每次计算机思考的时间，单位是秒
  int t_num;  //程序开始时设定的线程数量
  char color1;
  char color2;

  //初始化棋盘状态信息
  initBoard();

  //初始化随机数生成器
  initRandom();

  if(argc == 5)
  {
      s_time = atof(argv[1]);
      t_num = atoi(argv[2]);
      color1 = atoi(argv[3]);
      color2 = atoi(argv[4]);
  }
  else
  {
      s_time = SIM_TIME;
      t_num = THREAD_NUMBER;
      color1 = BLACK;
      color2 = WHITE;
  }

  boardInfo *bi = (boardInfo *)malloc(sizeof(boardInfo) * t_num);

  for(int i = 0; i < t_num; i++)
  {
      bi[i].id = i;
      bi[i].seed = getRandom(MAX_UINT) + i;
      bi[i].s_time = s_time;
      memset(bi[i].result_success, 0, sizeof(bi[i].result_success));
      memset(bi[i].result_failure, 0, sizeof(bi[i].result_failure));
      restoreBoard(&bi[i]);
      thread_init(&bi[i]);
  }

  int step = 0;
  int your_turn;
  uct_t *root;
  uct_t *uct_node;

  root = (uct_t)malloc(sizeof(uct_t));
  init_rootnode(root);

  //color1 表示对方的颜色
  //color2 表示计算机的颜色
  if(color1 == BLACK)
  {
    your_turn = FIRST;
  }
  else
  {
    your_turn = SECOND;
  }

  for(; ;)
  {
    if(your_turn == FIRST)
    {
      printf("It's my turn.\n");
      printf("I'm thinking ...\n");
      if(uct_node != NULL)
      {
        root = uct_node;
      }
      for(int i = 0; i < t_num; i++)
      {
        thread_start(&bi[i], root);
      }
      your_turn = SECOND;
    }
    else
    {
      printf("It's your turn.\n");
      printf("I'm waiting.\n");
      uct_node = node_create(pos);
      your_turn = FIRST;
    }
    step++;
  }
  
  return 0;
}

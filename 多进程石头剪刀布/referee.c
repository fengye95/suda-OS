/*
裁判进程： 通知选手出拳， 并判决
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include "defs.h"

// 定义消息结构体
typedef struct
{
    long type; // 类型： 记录谁发的
    int mv;    // 动作： 0 1 2  代表 石头剪刀布
} MSG;
key_t key;  // 定义消息队列的key
int msq_id; // 消息队列id

// 模拟裁判： 选手出拳并记时， 超时判负（返回-1）
int play_and_timeout(int player_id);

// 判断并输出输赢情况
int judge(int x, int y);

// 输出本轮结果
void output_result(int round, int ans);

// 全局变量
const char *moves[] = {"石头", "剪刀", "布"}; // 详细输出出拳信息

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s number\n", argv[0]);
    }
    
    printf("裁判进程启动\n");
    printf("当前工作目录: %s\n", get_working_dir());
    
    int T = atoi(argv[1]);
    
    // 初始化随机种子
    srand(time(NULL));
    
    // 创建消息队列
    if ((msq_id = create_game_msgq()) == -1) {
        return 1;
    }

    printf("===== 石头剪刀布对战游戏 =====\n");

    // 裁判进程
    int x, y, ans;
    int x_win = 0, y_win = 0;
    int timeout = 8;
    for (int i = 1; i <= T; i++)
    {
        printf("\n===== 第%d轮结果 =====\n", i);

        // 通知选手出拳并记时， 超时判负(返回-1)
        x = play_and_timeout(2);
        y = play_and_timeout(1);

        // 判断并输出输赢情况
        ans = judge(x, y);

        // 记录输赢总数
        x_win += (ans & 1);
        y_win += (ans >> 1) & 1;

        // 输出本轮输赢情况
        output_result(i, ans);
    }

    // 删除消息队列
    cleanup_msgq();

    // 输出最终结果
    printf("\n=== 最终结果 ===\n");
    printf("选手1获胜: %d 次\n", x_win);
    printf("选手2获胜: %d 次\n", y_win);
    printf("平局: %d 次\n", T - x_win - y_win);
    exit(0);

    return 0;
}

// 模拟裁判： 选手出拳并记时， 超时判负（返回-1）
int play_and_timeout(int player_id)
{
    int mv;

    // 发送消息
    MSG snd_msg = {player_id, 0};
    msgsnd(msq_id, &snd_msg, sizeof(MSG) - sizeof(long), 0);
    clock_t start = clock();
    // printf("DEBUG: 已向选手%d发送消息，等待回复...\n", player_id);

    // 接受消息
    MSG rcv_msg;
    msgrcv(msq_id, &rcv_msg, sizeof(MSG) - sizeof(long), player_id + 4, 0);
    clock_t time_p = clock() - start;

    mv = rcv_msg.mv;

    // 详细输出出拳信息
    printf("选手%d: ", player_id);
    if (mv == -1)
    {
        printf("超时 (耗时: %ld ticks)\n", time_p);
    }
    else
    {
        printf("%s (耗时: %ld ticks)\n", moves[mv], time_p);
    }

    return mv;
}

// 判断并输出输赢情况
int judge(int x, int y)
{
    // 平局
    if (x == y)
        return 0;

    // 选手1获胜
    if (y == -1 || (x + 1) % 3 == y)
        return 1;

    // 选手2获胜
    if (x == -1 || (y + 1) % 3 == x)
        return 2;
}

// 输出本轮结果
void output_result(int round, int ans)
{
    if (ans == 0)
    {
        printf("  双方平局！\n");
    }
    if (ans == 1)
    {
        printf("  选手1获胜！\n");
    }
    if (ans == 2)
    {
        printf("  选手2获胜！\n");
    }
}
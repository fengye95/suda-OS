/*
游戏规则： 两选手猜拳， 石头0 剪刀1 布2
          显然， x + 1 == y (mod 3) 的情况下， x获胜

三个进程：
    裁判进程
    选手1进程
    选手2进程

业务逻辑：
    裁判分别通知两选手出拳， 若在规定时间没有完成出拳判负
    记录每轮的输赢情况并输出
    最终输出总体输赢情况
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

// 模拟选手
void player(int pf[2], int ps[2]);

// 模拟裁判： 选手出拳并记时， 超时判负（返回-1）
int play_and_timeout(int player_id, int pf[2], int ps[2], int timeout);

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

    int T = atoi(argv[1]);
    int pf1[2], pf2[2];
    int ps1[2], ps2[2];
    pipe(pf1), pipe(pf2);
    pipe(ps1), pipe(ps2);

    // 两个选手进程
    if (fork() == 0)
    {
        close(ps1[1]);
        close(ps2[1]);
        // 选手进程1
        if (fork() == 0)
        {
            player(pf1, ps1);
            exit(0);
        }

        // 选手进程2
        if (fork() == 0)
        {
            player(pf2, ps2);
            exit(0);
        }

        while (wait(0) > 0)
            ;
        exit(0);
    }
    else
    {
        printf("===== 石头剪刀布对战游戏 =====\n");

        // 裁判进程 父进程
        int x, y, ans;
        int x_win = 0, y_win = 0;
        int timeout = 8;
        for (int i = 1; i <= T; i++)
        {
            printf("\n===== 第%d轮结果 =====\n", i);

            // 通知选手出拳并记时， 超时判负(返回-1)
            x = play_and_timeout(1, pf1, ps1, timeout);
            y = play_and_timeout(2, pf2, ps2, timeout);

            // 判断并输出输赢情况
            ans = judge(x, y);

            // 记录输赢总数
            x_win += (ans & 1);
            y_win += (ans >> 1) & 1;

            // 输出本轮输赢情况
            output_result(i, ans);
        }
        close(ps1[1]);
        close(ps2[1]);
        wait(0);
        // 输出最终结果
        printf("\n=== 最终结果 ===\n");
        printf("选手1获胜: %d 次\n", x_win);
        printf("选手2获胜: %d 次\n", y_win);
        printf("平局: %d 次\n", T - x_win - y_win);
        exit(0);
    }

    return 0;
}

// 模拟选手
void player(int pf[2], int ps[2])
{
    srand(time(0) ^ getpid());
    int x;
    while (read(ps[0], &x, 1) > 0)
    {
        x = rand() % 3;
        write(pf[1], &x, 4);
    }
}

// 模拟裁判： 选手出拳并记时， 超时判负（返回-1）
int play_and_timeout(int player_id, int pf[2], int ps[2], int timeout)
{
    int x;
    write(ps[1], "a", 1);
    clock_t start = clock();
    read(pf[0], &x, 4);
    clock_t time_p = clock() - start;

    if (time_p > timeout)
        x = -1;

    // 详细输出出拳信息
    printf("选手%d: ", player_id);
    if (x == -1)
    {
        printf("超时 (耗时: %ld ticks)\n", time_p);
    }
    else
    {
        printf("%s (耗时: %ld ticks)\n", moves[x], time_p);
    }
    // printf("time: %ld\n", ti);
    return x;
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
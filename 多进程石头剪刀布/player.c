/*
游戏规则： 两选手猜拳， 石头0 剪刀1 布2
          显然， x + 1 == y (mod 3) 的情况下， x获胜
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/msg.h>

// 定义消息结构体
typedef struct
{
    long type; // 类型： 记录谁发的
    int mv;    // 动作： 0 1 2  代表 石头剪刀布
} MSG;
key_t key;  // 定义消息队列的key
int msq_id; // 消息队列id

// 全局变量
const char *moves[] = {"石头", "剪刀", "布"}; // 详细输出出拳信息

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <player_id in {0, 1}>\n", argv[0]);
    }

    int player_id = atoi(argv[1]);

    // 获取消息队列
    if ((msq_id = get_game_msgq()) == -1) {
        return 1;
    }

    MSG snd_msg, rcv_msg;

    srand(time(0) ^ getpid());
    while (msgrcv(msq_id, &rcv_msg, sizeof(MSG) - sizeof(long), player_id, 0) > 0)
    {
        // printf("DEBUG: 已向裁判发送消息，等待回复...\n");

        int mv = rand() % 3;
        snd_msg = (MSG){player_id + 4, mv};
        msgsnd(msq_id, &snd_msg, sizeof(MSG) - sizeof(long), 0);
    }

    return 0;
}

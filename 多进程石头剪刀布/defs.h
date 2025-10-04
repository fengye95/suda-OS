#ifndef DEFS_H
#define DEFS_H

#include <sys/ipc.h>
#include <sys/msg.h>

#define MSGQ_DIR_NAME ".game_msgq"
#define MSGQ_ID_FILE "msgq_id"
#define MAX_KEY_ATTEMPTS 26

// 消息队列管理器函数
int create_game_msgq();           // 创建消息队列（裁判进程用）
int get_game_msgq();              // 获取消息队列（玩家进程用）
void cleanup_msgq();              // 清理消息队列
int is_msgq_valid(int msq_id);    // 检查消息队列是否有效

// 辅助函数
char* get_working_dir();          // 获取当前工作目录的绝对路径
char* get_msgq_dir_path();        // 获取消息队列目录的绝对路径
char* get_msgq_file_path();       // 获取消息队列ID文件的绝对路径

#endif // DEFS_H
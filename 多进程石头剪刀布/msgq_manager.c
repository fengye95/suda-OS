#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <limits.h>  // 添加这个头文件
// 调试：检查 PATH_MAX 是否定义

    #define PATH_MAX 4096


static int write_msgq_id(int msq_id); // 写入消息队列ID到文件

static int read_msgq_id() ; // 从文件读取消息队列ID

static void ensure_msgq_dir() ; // 确保消息队列目录存在

// 获取工作目录的绝对路径
char* get_working_dir() {
    static char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        return cwd;
    } else {
        perror("getcwd");
        return ".";
    }
}

// 获取消息队列目录的绝对路径
char* get_msgq_dir_path() {
    static char dir_path[PATH_MAX];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", get_working_dir(), MSGQ_DIR_NAME);
    return dir_path;
}

// 获取消息队列ID文件的绝对路径
char* get_msgq_file_path() {
    static char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "%s/%s", get_msgq_dir_path(), MSGQ_ID_FILE);
    return file_path;
}

// 创建消息队列（裁判进程调用）
int create_game_msgq() {
    printf("正在初始化消息队列...\n");
    printf("工作目录: %s\n", get_working_dir());

    // 先清理可能存在的旧消息队列
    cleanup_msgq();
    
    // 尝试多个key创建消息队列
    char key_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int used_keys[MAX_KEY_ATTEMPTS] = {0};
    
    for (int attempt = 0; attempt < MAX_KEY_ATTEMPTS; attempt++) {
        // 随机选择key字符，避免冲突
        int key_index;
        do {
            key_index = rand() % 26;
        } while (used_keys[key_index] && attempt < MAX_KEY_ATTEMPTS - 1);
        used_keys[key_index] = 1;
        
        // 使用绝对路径生成key
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/referee.c", get_working_dir());
        
        key_t key = ftok(path, key_chars[key_index]);
        if (key == -1) {
            printf("  尝试 key %c: ftok失败\n", key_chars[key_index]);
            continue;
        }
        
        // 检查是否已存在，直接尝试下一个key
        int existing_id;
        if ((existing_id = msgget(key, 0666)) != -1) {
            printf("  消息队列已存在(ID=%d)，跳过 key %c\n", existing_id, key_chars[key_index]);
            continue;  // 直接尝试下一个key
        }
        
        // 创建新消息队列
        int msq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0777);
        if (msq_id != -1) {
            printf("  成功创建消息队列: ID=%d, key=%c\n", msq_id, key_chars[key_index]);
            
            // 写入文件
            if (write_msgq_id(msq_id) == 0) {
                printf("  消息队列ID已保存到: %s\n", get_msgq_file_path());
                return msq_id;
            } else {
                // 写入失败，清理消息队列
                msgctl(msq_id, IPC_RMID, NULL);
                printf("  保存消息队列ID失败，已清理\n");
            }
        } else {
            printf("  尝试 key %c: 创建失败 (%s)\n", key_chars[key_index], strerror(errno));
        }
    }
    
    fprintf(stderr, "错误: 无法创建消息队列，所有尝试都失败了\n");
    return -1;
}

// 获取消息队列（玩家进程调用）
int get_game_msgq() {
    int msq_id = read_msgq_id();
    if (msq_id == -1) {
        fprintf(stderr, "错误: 无法读取消息队列ID文件\n");
        return -1;
    }
    
    if (!is_msgq_valid(msq_id)) {
        fprintf(stderr, "错误: 消息队列 %d 无效或不存在\n", msq_id);
        // 清理无效的文件
        cleanup_msgq();
        return -1;
    }
    
    printf("成功获取消息队列: ID=%d\n", msq_id);
    return msq_id;
}

// 清理消息队列
void cleanup_msgq() {
    int msq_id = read_msgq_id();
    if (msq_id != -1) {
        if (is_msgq_valid(msq_id)) {
            if (msgctl(msq_id, IPC_RMID, NULL) == 0) {
                printf("已释放消息队列: ID=%d\n", msq_id);
            } else {
                printf("释放消息队列失败: %s\n", strerror(errno));
            }
        } else {
            printf("消息队列 %d 已不存在\n", msq_id);
        }
    }
    
    // 删除ID文件
    if (remove(get_msgq_file_path()) == 0) {
        printf("已删除消息队列ID文件\n");
    } else if (errno != ENOENT) {
        printf("删除消息队列ID文件失败: %s\n", strerror(errno));
    }
    
    // 尝试删除目录（如果为空）
    rmdir(get_msgq_dir_path());
}

// 检查消息队列是否有效
int is_msgq_valid(int msq_id) {
    struct msqid_ds buf;
    return (msgctl(msq_id, IPC_STAT, &buf) != -1);
}

// ========== 内部函数 ==========

// 写入消息队列ID到文件
static int write_msgq_id(int msq_id) {
    ensure_msgq_dir();
    
    FILE *file = fopen(get_msgq_file_path(), "w");
    if (file == NULL) {
        perror("fopen write");
        return -1;
    }
    fprintf(file, "%d", msq_id);
    fclose(file);
    return 0;
}

// 从文件读取消息队列ID
static int read_msgq_id() {
    FILE *file = fopen(get_msgq_file_path(), "r");
    if (file == NULL) {
        return -1;
    }
    int msq_id;
    if (fscanf(file, "%d", &msq_id) != 1) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return msq_id;
}

// 确保消息队列目录存在
static void ensure_msgq_dir() {
    struct stat st = {0};
    if (stat(get_msgq_dir_path(), &st) == -1) {
        if (mkdir(get_msgq_dir_path(), 0755) == -1) {
            perror("mkdir");
        }
    }
}
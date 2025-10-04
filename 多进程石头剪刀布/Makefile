# 编译器设置
CC = gcc
CFLAGS = -std=c11 -g
LDFLAGS = 

# 目标文件
TARGETS = referee player
OBJS = msgq_manager.o

# 默认目标
all: $(TARGETS)

test: $(TARGETS)
test: $(TARGETS)
	@echo "在tmux中启动三个面板..."
	tmux kill-session -t game 2>/dev/null || true
	tmux new-session -d -s game 'bash -c "./referee 10; read -p \"按回车键退出...\""'
	tmux split-window -h -t game 'bash -c "./player 1; read -p \"按回车键退出...\" " '
	tmux split-window -v -t game 'bash -c "./player 2; read -p \"按回车键退出...\" " '
	tmux attach -t game

# 裁判进程
referee: referee.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 玩家进程  
player: player.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 消息队列管理器
msgq_manager.o: msgq_manager.c defs.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 源文件依赖
referee.o: referee.c defs.h
	$(CC) $(CFLAGS) -c -o $@ $<

player.o: player.c defs.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理
clean:
	rm -f $(TARGETS) *.o
	rm -rf .game_msgq



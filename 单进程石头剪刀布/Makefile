# 定义编译器和编译目标
CC = gcc
TARGETS = main_msq main_pipe  # 所有可执行文件列表

# 模式规则：自动匹配所有 .c 源文件，编译为同名可执行文件
%: %.c
	$(CC) $< -o $@  

# 清理所有生成的可执行文件
clean:
	rm -rf $(TARGETS)
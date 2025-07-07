# @Author: Yuzhe Guo
# @Date: 2025-07-07 14:42:12
# @FilePath: /OS/linux_shell/Makefile
# @Descripttion: 编译脚本

# 编译器
CC = gcc

# 编译选项
# -g: 添加调试信息
# -Wall: 显示所有警告
CFLAGS = -Wall -g

# 头文件目录
INCDIR = include
# 添加头文件目录到 CFLAGS
CFLAGS += -I$(INCDIR)

# 源文件目录
SRCDIR = src
# 目标文件（.o 文件）目录
OBJDIR = obj

# 最终生成的可执行文件名
TARGET = myshell

# 自动查找 src 目录下的所有 .c 文件
SRCS := $(wildcard $(SRCDIR)/*.c)

# 根据 .c 文件列表，自动生成对应的 .o 文件路径列表
# 例如，将 src/main.c 映射为 obj/main.o
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# 链接时需要使用的库（例如 readline）
# LDFLAGS = -lreadline

# --- 规则 ---

# 默认规则：第一个规则是 'make' 命令默认执行的规则
all: $(TARGET)

# 链接规则：如何从所有的 .o 文件生成最终的可执行文件
$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "Build finished: $(TARGET)"

# 编译规则（模式规则）: 如何从一个 .c 文件生成一个 .o 文件
# $@ 代表目标文件 (例如 obj/main.o)
# $< 代表第一个依赖文件 (例如 src/main.c)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR) # 确保 obj 目录存在
	@echo "Compiling $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR) $(TARGET)

# 将 all 和 clean 声明为“伪目标”
.PHONY: all clean
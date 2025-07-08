/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:58:34
 * @FilePath: /linux-shell/include/shell.h
 * @Descripttion: 项目总头文件,定义所有模块共享的数据结构和函数原型
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> // for open flags

// 常量定义
#define MAX_CMD_LEN 1024 // 最大命令长度
#define MAX_ARGS 64      // 最大参数数量
#define HIST_SIZE 20     // 添加一个宏，用于定义命令历史记录大小

// 命令结构体，用于存储解析后的命令
// 这一步对于实现管道和重定向至关重要
typedef struct {
    char* args[MAX_ARGS];   // 参数列表，以 NULL 结尾
    char* input_file;       // 输入重定向文件
    char* output_file;      // 输出重定向文件
    int is_background;      // 是否后台执行
} command_t;


// 在文件顶部或合适的位置，定义 Alias 结构体
typedef struct Alias {
    char* name;
    char* command;
    struct Alias* next;
} Alias;


// 函数原型
// parser.c
int parse_command(char* line, command_t* cmd);
int parse_pipe_commands(char* line, command_t* cmds, int* cmd_count);

// execute.c
void execute_command(command_t* cmd);
void execute_pipeline(command_t* cmds, int cmd_count);

// builtins.c
int handle_builtin_command(command_t* cmd);
void builtin_cd(char** args);
void builtin_echo(char** args);
void builtin_type(char** args);

void builtin_alias(char** args);
void builtin_unalias(char** args);  // 新增
char* expand_alias(char* line);     // 新增，这个函数非常关键

// 添加和修改以下history函数原型
void add_to_history(const char* cmd); // 新增
void builtin_history(char** args);    // 修改，确保参数统一
// history system getters
int get_history_count();
const char* get_history_entry(int index);

// 添加新函数的原型completion.c
void initialize_completion();
char** completion_callback(const char* text, int start, int end);

// main.c
void main_loop();
void display_prompt();

#endif // SHELL_H
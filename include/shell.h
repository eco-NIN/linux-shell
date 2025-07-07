/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:58:34
 * @FilePath: /OS/linux_shell/src/shell.h
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

// 命令结构体，用于存储解析后的命令
// 这一步对于实现管道和重定向至关重要
typedef struct {
    char* args[MAX_ARGS];   // 参数列表，以 NULL 结尾
    char* input_file;       // 输入重定向文件
    char* output_file;      // 输出重定向文件
    int is_background;      // 是否后台执行
} command_t;

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
void builtin_history();
void builtin_type(char** args);
void builtin_alias(char** args);

// main.c
void main_loop();
void display_prompt();

#endif // SHELL_H
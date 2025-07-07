/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:51:12
 * @FilePath: /linux-shell/src/parser.c
 * @Descripttion: 命令解析模块,这个模块负责将字符串拆分成 command_t 结构。我们先实现一个简单的版本，处理空格分隔的参数
 */

#include "shell.h"
#include <stdbool.h> // 引入布尔类型，让代码更清晰

int parse_command(char* line, command_t* cmd) {
    // 初始化 command_t 结构体
    // 确保每次解析前都是干净的状态
    memset(cmd, 0, sizeof(command_t)); 

    char* line_copy = strdup(line);
    if (line_copy == NULL) {
        perror("strdup");
        return 0;
    }

    char* token;
    int arg_count = 0;

    // 第一次遍历：用 strtok 分割参数
    token = strtok(line_copy, " \t\r\n\a");
    while(token != NULL && arg_count < MAX_ARGS - 1) {
        cmd->args[arg_count++] = strdup(token);
        token = strtok(NULL, " \t\r\n\a");
    }
    cmd->args[arg_count] = NULL;
    
    // 第二次遍历：处理特殊符号（重定向、后台）
    // 这个循环从前往后，找到第一个特殊符号并处理后就应该停止
    // 否则会错误处理像 "cmd > file1 > file2" 这样的命令
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(cmd->args[i], ">") == 0) {
            // 检查语法错误: `ls >`
            if (cmd->args[i+1] == NULL) {
                fprintf(stderr, "myshell: syntax error near unexpected token `newline'\n");
                cmd->args[0] = NULL; // 将命令标记为无效，防止执行
                break;
            }
            cmd->output_file = cmd->args[i+1];
            cmd->args[i] = NULL; // 从参数列表中移除
            // cmd->args[i+1] = NULL; // 也可以把文件名也移除
        } else if (strcmp(cmd->args[i], "<") == 0) {
            if (cmd->args[i+1] == NULL) {
                fprintf(stderr, "myshell: syntax error near unexpected token `newline'\n");
                cmd->args[0] = NULL; // 标记为无效
                break;
            }
            cmd->input_file = cmd->args[i+1];
            cmd->args[i] = NULL;
        } else if (strcmp(cmd->args[i], "&") == 0) {
            cmd->is_background = 1;
            cmd->args[i] = NULL;
        }
    }

    free(line_copy);

    if (cmd->args[0] == NULL) return 0; // 如果是无效命令，返回0

    return arg_count;
}
// 在 parser.c 中添加
int parse_pipe_commands(char* line, command_t* cmds, int* cmd_count) {
    char* next_cmd = line;
    char* pipe_pos;
    int count = 0;


    // 使用 strsep 来分割，比 strtok 更安全
    while ((pipe_pos = strchr(next_cmd, '|')) != NULL) {
        *pipe_pos = '\0'; // 用 NULL 分割命令
        parse_command(next_cmd, &cmds[count++]);
        next_cmd = pipe_pos + 1;
    }
    parse_command(next_cmd, &cmds[count++]);
    
    *cmd_count = count;
    return count;
}
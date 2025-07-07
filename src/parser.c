/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:51:12
 * @FilePath: /linux-shell/src/parser.c
 * @Descripttion: 命令解析模块,这个模块负责将字符串拆分成 command_t 结构。我们先实现一个简单的版本，处理空格分隔的参数
 */

//真正的根本原因：parser.c 的“过度”解析

// 问题出在我们项目最基础的模块之一：parser.c 中的 parse_command 函数。

// 我们用 strtok 函数以空格为分隔符来拆分命令。这个方法过于简单，它不理解引号的含义。当它看到 alias ll='ls -alF' 这条命令时，它的行为是：

// 找到第一个词 alias。

// 找到第二个词 ll='ls (它在空格处停下，将 'ls 和 -alF' 分割开)。

// 找到第三个词 -alF'。

// 所以，最终传递给 builtin_alias 函数的 args 数组，其内容实际上是：

// args[0] = "alias"

// args[1] = "ll='ls"  <-- 问题所在

// args[2] = "-alF'" <-- 问题所在

// args[3] = NULL

// 当我之前编写 builtin_alias 的修复代码时，我错误地假设了 args[1] 会是完整的 "ll='ls -alF'"。但实际上，它已经被我们的解析器无情地拆开了。

// 因此，builtin_alias 只拿到了 ll='ls 进行处理，自然就错误地将 'ls 存为了别名的值。这就完美解释了你看到的所有现象。


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
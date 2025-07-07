/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:51:12
 * @FilePath: /OS/linux_shell/src/parser.c
 * @Descripttion: 命令解析模块,这个模块负责将字符串拆分成 command_t 结构。我们先实现一个简单的版本，处理空格分隔的参数
 */

#include "shell.h"

// 在 parser.c 中修改 parse_command
int parse_command(char* line, command_t* cmd) {
    // ... (初始化代码) ...
    char* line_copy = strdup(line); // 复制一份，因为我们要修改它
    char* token;
    int arg_count = 0;

    // 分割，但先不处理特殊字符
    token = strtok(line_copy, " \t\r\n\a");
    while(token != NULL) {
        cmd->args[arg_count++] = strdup(token);
        token = strtok(NULL, " \t\r\n\a");
    }
    cmd->args[arg_count] = NULL;
    
    // 现在从后向前检查特殊字符
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(cmd->args[i], ">") == 0) {
            cmd->output_file = cmd->args[i+1];
            cmd->args[i] = NULL; // 从参数列表中移除
        } else if (strcmp(cmd->args[i], "<") == 0) {
            cmd->input_file = cmd->args[i+1];
            cmd->args[i] = NULL;
        } else if (strcmp(cmd->args[i], "&") == 0) {
            cmd->is_background = 1;
            cmd->args[i] = NULL;
        }
    }

    free(line_copy);
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
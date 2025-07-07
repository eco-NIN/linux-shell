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

// 在 src/parser.c 中
// ==========================================================
//            !!! 全新的、支持引号的 parse_command !!!
// ==========================================================
int parse_command(char* line, command_t* cmd) {
    memset(cmd, 0, sizeof(command_t));
    int argc = 0;
    char* buf = line;
    char* arg_start = NULL;
    int in_quote = 0;

    while (*buf != '\0' && argc < MAX_ARGS - 1) {
        // 跳过前导的空白字符
        while (*buf == ' ' || *buf == '\t') {
            buf++;
        }

        if (*buf == '\0') break; // 到达行尾

        // 检查是否是引号
        if (*buf == '\"') {
            in_quote = 1;
            buf++; // 跳过开头的引号
            arg_start = buf;
            // 寻找匹配的结束引号
            while (*buf != '\0' && *buf != '\"') {
                buf++;
            }
        } else {
            in_quote = 0;
            arg_start = buf;
            // 寻找下一个空白字符
            while (*buf != '\0' && *buf != ' ' && *buf != '\t') {
                buf++;
            }
        }

        if (*buf != '\0') {
            if (in_quote) {
                if (*buf != '\"') {
                     fprintf(stderr, "myshell: syntax error: unclosed quote\n");
                     return 0;
                }
            }
            *buf = '\0'; // 用 NULL 截断，得到一个完整的参数
            buf++;       // 移动到下一个位置
        }
        
        cmd->args[argc++] = strdup(arg_start);
    }
    cmd->args[argc] = NULL;
    
    // 处理重定向的逻辑（可以保持不变，但要确保它在新的解析器下正常工作）
    for (int i = 0; i < argc; i++) {
        if (cmd->args[i] != NULL && strcmp(cmd->args[i], ">") == 0) {
            if (cmd->args[i+1] != NULL) {
                cmd->output_file = cmd->args[i+1];
                cmd->args[i] = NULL;
            }
        } else if (cmd->args[i] != NULL && strcmp(cmd->args[i], "<") == 0) {
            // ... (类似的处理) ...
        } // ... 等等
    }

    return argc;
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


// // 在 src/parser.c 中
// // ==========================================================
// // 调试版本：parse_pipe_commands
// // ==========================================================
// int parse_pipe_commands(char* line, command_t* cmds, int* cmd_count) {
//     printf("\n--- [DEBUG] Entering parse_pipe_commands with line: \"%s\"\n", line);

//     char* next_cmd = line;
//     char* pipe_pos;
//     int count = 0;

//     while ((pipe_pos = strchr(next_cmd, '|')) != NULL) {
//         *pipe_pos = '\0';
//         printf("--- [DEBUG] Pipe found. Parsing command part: \"%s\"\n", next_cmd);
//         parse_command(next_cmd, &cmds[count++]);
//         next_cmd = pipe_pos + 1;
//         // Trim leading spaces for the next command part
//         while (*next_cmd == ' ') next_cmd++;
//     }
//     printf("--- [DEBUG] No more pipes. Parsing final command part: \"%s\"\n", next_cmd);
//     parse_command(next_cmd, &cmds[count++]);
    
//     *cmd_count = count;
//     printf("--- [DEBUG] Leaving parse_pipe_commands. Found %d commands.\n\n", *cmd_count);
//     return count;
// }


// // ==========================================================
// // 调试版本：parse_command
// // ==========================================================
// int parse_command(char* line, command_t* cmd) {
//     memset(cmd, 0, sizeof(command_t));
//     char* line_copy = strdup(line);
//     if (!line_copy) {
//         perror("strdup");
//         return 0;
//     }

//     char* token;
//     int arg_count = 0;

//     token = strtok(line_copy, " \t\r\n\a");
//     while(token != NULL && arg_count < MAX_ARGS - 1) {
//         cmd->args[arg_count++] = strdup(token);
//         token = strtok(NULL, " \t\r\n\a");
//     }
//     cmd->args[arg_count] = NULL;
    
//     // 打印出解析结果
//     printf("    [DEBUG] parse_command result for \"%s\":\n", line);
//     for (int i = 0; i < arg_count; i++) {
//         printf("        args[%d]: %s\n", i, cmd->args[i]);
//     }

//     // 处理重定向等（这部分代码保持不变）
//     for (int i = 0; i < arg_count; i++) {
//         if (cmd->args[i] != NULL && strcmp(cmd->args[i], ">") == 0) {
//             if (cmd->args[i+1] != NULL) {
//                 cmd->output_file = cmd->args[i+1];
//                 cmd->args[i] = NULL;
//             }
//         } else if (cmd->args[i] != NULL && strcmp(cmd->args[i], "<") == 0) {
//             if (cmd->args[i+1] != NULL) {
//                 cmd->input_file = cmd->args[i+1];
//                 cmd->args[i] = NULL;
//             }
//         } else if (cmd->args[i] != NULL && strcmp(cmd->args[i], "&") == 0) {
//             cmd->is_background = 1;
//             cmd->args[i] = NULL;
//         }
//     }

//     free(line_copy);
//     return arg_count;
// }
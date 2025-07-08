/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:42:31
 * @FilePath: /linux-shell/src/main.c
 * @Descripttion: 主程序入口和主循环
 */

// 初始化(Initialize)：加载配置（如历史记录、别名）。

// 读取(Read)：显示提示符（例如 myshell> ），并读取用户输入的一整行命令。

// 解析(Parse)：将用户输入的字符串分解成程序可以理解的结构（例如，命令名、参数、重定向符号等）。

// 执行(Execute)：根据解析结果，执行相应的命令（内建命令或外部程序）。（如果是内部的话，会调用builtins.c中的内建命令函数）

// 循环(Loop)：回到第2步，等待下一条命令。

#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h> // 为了 gethostname
#include <ctype.h> // for isspace()
#include <stdbool.h> // for bool type
// 定义包含了 \001 和 \002 的、readline 安全的 ANSI 颜色代码
#define C_RESET   "\001\033[0m\002"
#define C_BLACK   "\001\033[30m\002"
#define C_RED     "\001\033[31m\002"
#define C_GREEN   "\001\033[32m\002"
#define C_YELLOW  "\001\033[33m\002"
#define C_BLUE    "\001\033[34m\002"
#define C_MAGENTA "\001\033[35m\002"
#define C_CYAN    "\001\033[36m\002"
#define C_WHITE   "\001\033[37m\002"

// 函数原型
void main_loop();
char* get_prompt();
void initialize_shell();
int parse_line(char* line, command_t* cmds);

int main() {
    initialize_shell();
    main_loop();
    return EXIT_SUCCESS;
}

/**
 * @description: 初始化 Shell，包括命令补全
 */
void initialize_shell() {
    // 绑定我们的补全回调函数
    // 向 readline 注册自己的补全处理函数 completion_callback
    // 这个函数定义在 completion.c 中
    rl_attempted_completion_function = completion_callback;
}

/**
 * @description: Shell 的主循环，使用 readline
 */
void main_loop() {
    char* line_from_readline; // 从 readline 读取的原始行
    char* line_to_process;    // 经过历史展开后，最终要处理的行
    command_t cmds[MAX_ARGS];
    int cmd_count;

    while (1) {
        char* prompt = get_prompt();
        line_from_readline = readline(prompt);
        free(prompt);

        if (line_from_readline == NULL) { // Ctrl+D
            printf("exit\n");
            break;
        }

        // 如果是空行，释放内存并开始下一次循环
        if (!*line_from_readline) {
            free(line_from_readline);
            continue;
        }

        bool expansion_failed = false;

        // --- 新增：在这里处理历史命令展开 ---
        if (line_from_readline[0] == '!') {
            const char* history_cmd = NULL;

            // 处理 '!!'
            if (line_from_readline[1] == '!' && (line_from_readline[2] == '\0' || isspace(line_from_readline[2]))) {
                history_cmd = get_history_entry(get_history_count() - 1);
            } 
            // 处理 '!n'
            else {
                int n = atoi(&line_from_readline[1]);
                if (n > 0) {
                    history_cmd = get_history_entry(n - 1); // 我们的历史数组索引从 0 开始
                }
            }

            if (history_cmd) {
                line_to_process = strdup(history_cmd); // 复制历史命令
                printf("%s\n", line_to_process);     // 回显到屏幕
            } else {
                fprintf(stderr, "myshell: %s: event not found\n", line_from_readline);
                expansion_failed = true;
            }
        } else {
            // 如果不是历史展开命令，则直接处理当前输入
            line_to_process = strdup(line_from_readline);
        }
        
        // 释放 readline 返回的原始行，我们现在只跟 line_to_process 打交道
        free(line_from_readline);
        
        // 如果历史展开失败，则跳过本次循环
        if (expansion_failed) {
            free(line_to_process); // 别忘了释放为它分配的内存
            continue;
        }
        
        // --- 原有的处理流程，现在都基于 line_to_process ---
        
        // 将最终要执行的命令添加到两个历史记录系统
        add_history(line_to_process);
        add_to_history(line_to_process);
        
        char* expanded_line = expand_alias(line_to_process);
        
        if (strlen(expanded_line) > 0) {
            cmd_count = parse_line(expanded_line, cmds);
            if (cmd_count > 0) {
                if (cmd_count > 1) {
                    execute_pipeline(cmds, cmd_count);
                } else {
                    if (handle_builtin_command(&cmds[0]) == 0 && cmds[0].args[0] != NULL) {
                        // 代码首先进入 if 的条件判断，执行 handle_builtin_command(&cmds[0])。
                        // handle_builtin_command 函数（在 builtins.c 中）会拿到 "cd" 这个名字。
                        // 它会在自己的内建命令列表 builtin_str[] 中进行查找。
                        // 它找到了！ "cd" 在列表里。于是，它立刻调用对应的 C 函数 builtin_cd(cmds[0].args)。
                        // builtin_cd() 函数直接在当前 Shell 进程内部执行 chdir("src") 系统调用，改变了 Shell 的工作目录。
                        
                        // ‼️
                        // builtin_cd() 执行完毕后，handle_builtin_command 函数返回 1（表示“我成功处理了这个命令”）。

                        // 回到 main.c，if 的条件变成了 if (1 == 0)，这个条件是假。
                        // 因此，if 代码块内部的 execute_command(&cmds[0]) 完全不会被执行。
                        //【路径 B】如果不是内建命令
                        execute_command(&cmds[0]);//执行外部命令
                    }
                    // 【路径 A】执行内建命令，main_loop 继续下一次循环
                }
            }
        }
        
        free(expanded_line);
        free(line_to_process); // 释放我们为 line_to_process 分配的内存
    }
}

// /**
//  * @description: 生成提示符字符串
//  * @return {char*} 返回一个需要被 free 的字符串
//  */
// char* get_prompt() {
//     char cwd[1024];
//     char* prompt = (char*)malloc(1024 + 32);
//     if (getcwd(cwd, sizeof(cwd)) != NULL) {
//         snprintf(prompt, 1024 + 32, "\033[1;32m%s\033[0m$ ", cwd);
//     } else {
//         snprintf(prompt, 1024 + 32, "myshell$ ");
//     }
//     return prompt;
// }

/**
 * @description: 生成一个完整、美化的命令提示符字符串
 * @return {char*} 返回一个需要被 free 的字符串
 */
char* get_prompt() {
    // --- 1. 获取基本信息 ---
    char hostname[256];
    char* user = getenv("USER");
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }

    // --- 2. 获取并处理路径 ---
    char cwd[1024];
    char path_display[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char* home_dir = getenv("HOME");
        // 如果当前路径以家目录开头，则用 '~' 替换
        if (home_dir && strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
            snprintf(path_display, sizeof(path_display), "~%s", cwd + strlen(home_dir));
        } else {
            strncpy(path_display, cwd, sizeof(path_display));
        }
    } else {
        strcpy(path_display, "unknown_path");
    }

    // --- 3. 拼接所有部分 ---
    char* prompt = (char*)malloc(2048); // 分配足够大的空间
    snprintf(prompt, 2048,
        "%s[linux-shell]%s %s%s%s@%s%s:%s%s%s$",
        C_YELLOW,                            // [myshell] 标识 (黄色)
        C_RESET,                             // 重置颜色
        C_GREEN, user ? user : "user",       // 用户名 (绿色)
        C_WHITE, hostname,              // @主机名 (白色)
        C_RESET,                             // 重置颜色
        C_CYAN, path_display,                // 路径 (青色)
        C_RESET                              // 重置颜色
    );
    
    return prompt;
}
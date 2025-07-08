
// // 初始化(Initialize)：加载配置（如历史记录、别名）。

// // 读取(Read)：显示提示符（例如 myshell> ），并读取用户输入的一整行命令。

// // 解析(Parse)：将用户输入的字符串分解成程序可以理解的结构（例如，命令名、参数、重定向符号等）。

// // 执行(Execute)：根据解析结果，执行相应的命令（内建命令或外部程序）。

// // 循环(Loop)：回到第2步，等待下一条命令。

/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:42:31
 * @FilePath: /linux-shell/src/main.c
 * @Descripttion: 主程序入口和主循环
 */
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

#include <unistd.h> // 为了 gethostname


// 定义 ANSI 颜色代码
// #define C_RESET   "\033[0m"
// #define C_BLACK   "\033[30m"
// #define C_RED     "\033[31m"
// #define C_GREEN   "\033[32m"
// #define C_YELLOW  "\033[33m"
// #define C_BLUE    "\033[34m"
// #define C_MAGENTA "\033[35m"
// #define C_CYAN    "\033[36m"
// #define C_WHITE   "\033[37m"
// ✅ 定义包含了 \001 和 \002 的、readline 安全的 ANSI 颜色代码
#define C_RESET   "\001\033[0m\002"
#define C_BLACK   "\001\033[30m\002"
#define C_RED     "\001\033[31m\002"
#define C_GREEN   "\001\033[32m\002"
#define C_YELLOW  "\001\033[33m\002"
#define C_BLUE    "\001\033[34m\002"
#define C_MAGENTA "\001\033[35m\002"
#define C_CYAN    "\001\033[36m\002"
#define C_WHITE   "\001\033[37m\002"

// 幸运的是，readline 库提供了另一套更通用、更推荐的“记号”，就是 \[ 和 \]。

// 它们的作用和 \001, \002 完全一样，都是用来包裹非打印字符序列，但它们的兼容性更好，是专门设计给用户在设置提示符字符串时使用的。

// \[: 相当于开始标记 \001

// \]: 相当于结束标记 \002

// #define C_RESET   "\[\033[0m\]"
// #define C_BLACK   "\[\033[30m\]"
// #define C_RED     "\[\033[31m\]"
// #define C_GREEN   "\[\033[32m\]"
// #define C_YELLOW  "\[\033[33m\]"
// #define C_BLUE    "\[\033[34m\]"
// #define C_MAGENTA "\[\033[35m\]"
// #define C_CYAN    "\[\033[36m\]"
// #define C_WHITE   "\[\033[37m\]"


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
    rl_attempted_completion_function = completion_callback;
}

/**
 * @description: Shell 的主循环，使用 readline
 */
void main_loop() {
    char* line;
    char* expanded_line;
    command_t cmds[MAX_ARGS];
    int cmd_count;

    while (1) {
        char* prompt = get_prompt();
        line = readline(prompt); // 使用 readline 读取输入
        free(prompt);

        if (line == NULL) { // 用户按下 Ctrl+D
            printf("exit\n");
            break;
        }

        if (line && *line) {
            add_history(line); // 使用 readline 自带的历史记录功能
            
            // ✅ 同时，也要把命令添加到我们自己的历史记录数组中 (为了让 `history` 命令能工作)
            add_to_history(line); 

            expanded_line = expand_alias(line);

            if (strlen(expanded_line) > 0) {
                // ✅ 替换为对新函数的调用
                cmd_count = parse_line(expanded_line, cmds);
                //cmd_count = parse_pipe_commands(expanded_line, cmds);
                //cmd_count = parse_pipe_commands(expanded_line, cmds, &cmd_count);

                if (cmd_count > 1) {
                    execute_pipeline(cmds, cmd_count);
                } else {
                    if (handle_builtin_command(&cmds[0]) == 0) {
                        if (cmds[0].args[0] != NULL)
                            execute_command(&cmds[0]);
                    }
                }
            }
            free(expanded_line);
        }
        free(line); // 每次循环后释放 readline 分配的内存
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

// 在 src/main.c 中

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
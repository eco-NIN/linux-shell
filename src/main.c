// /*
//  * @Author: Yuzhe Guo
//  * @Date: 2025-07-07 14:42:31
//  * @FilePath: /linux-shell/src/main.c
//  * @Descripttion: 主程序入口和主循环
//  */


// // 初始化(Initialize)：加载配置（如历史记录、别名）。

// // 读取(Read)：显示提示符（例如 myshell> ），并读取用户输入的一整行命令。

// // 解析(Parse)：将用户输入的字符串分解成程序可以理解的结构（例如，命令名、参数、重定向符号等）。

// // 执行(Execute)：根据解析结果，执行相应的命令（内建命令或外部程序）。

// // 循环(Loop)：回到第2步，等待下一条命令。



// #include "shell.h"

// int main() {
//     // 初始化工作，例如加载 .myshell_history, .myshell_aliases 等
//     // (我们后面再实现)

//     // 进入主循环
//     main_loop();

//     return EXIT_SUCCESS;
// }

// void main_loop() {
//     char line[MAX_CMD_LEN];
//     char* expanded_line;
//     command_t cmds[MAX_ARGS]; // 用于存储管道命令
//     int cmd_count;

//     while (1) {
//         display_prompt();

//         if (fgets(line, MAX_CMD_LEN, stdin) == NULL) {
//             // Ctrl+D a terminates the shell
//             printf("\n");
//             break;
//         }

//         // 移除换行符
//         line[strcspn(line, "\n")] = 0;

//         // !!! history集成点在这里 !!!
//         // 添加到历史记录的是原始命令
//         add_to_history(line);

//         // !!! 集成点：在解析前展开别名 !!!
//         expanded_line = expand_alias(line);


//         // 如果命令行为空（或只有空格），则继续
//         if (strlen(expanded_line) == 0) {
//             free(expanded_line);
//             continue;
//         }

//         // 解析管道
//         // 使用展开后的命令进行解析和执行
//         //cmd_count = parse_pipe_commands(line, cmds, &cmd_count);
//         cmd_count = parse_pipe_commands(expanded_line, cmds, &cmd_count);

//         if (cmd_count > 1) {
//             execute_pipeline(cmds, cmd_count);
//         } else {
//             // 解析单个命令 (包括重定向和后台)
//             parse_command(expanded_line, &cmds[0]);
            
//             if (cmds[0].args[0] != NULL) {
//                 // 尝试作为内建命令处理
//                 if (handle_builtin_command(&cmds[0]) == 0) {
//                     // 如果不是内建命令，则作为外部命令执行
//                     execute_command(&cmds[0]);
//                 }
//             }
//         }

//         // !!! 关键：释放 expand_alias 分配的内存 !!!
//         free(expanded_line);
//     }
// }

// void display_prompt() {
//     char cwd[1024];
//     if (getcwd(cwd, sizeof(cwd)) != NULL) {
//         printf("\033[1;32m%s\033[0m$ ", cwd); // 绿色显示当前路径
//     } else {
//         perror("getcwd() error");
//         printf("myshell$ ");
//     }
//     fflush(stdout);
// }





// ============== 新===============

/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:42:31
 * @FilePath: /linux-shell/src/main.c
 * @Descripttion: 主程序入口和主循环
 */
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

// 函数原型
void main_loop();
char* get_prompt();
void initialize_shell();

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
            
            expanded_line = expand_alias(line);

            if (strlen(expanded_line) > 0) {
                //cmd_count = parse_pipe_commands(expanded_line, cmds);
                cmd_count = parse_pipe_commands(expanded_line, cmds, &cmd_count);

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

/**
 * @description: 生成提示符字符串
 * @return {char*} 返回一个需要被 free 的字符串
 */
char* get_prompt() {
    char cwd[1024];
    char* prompt = (char*)malloc(1024 + 32);
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, 1024 + 32, "\033[1;32m%s\033[0m$ ", cwd);
    } else {
        snprintf(prompt, 1024 + 32, "myshell$ ");
    }
    return prompt;
}
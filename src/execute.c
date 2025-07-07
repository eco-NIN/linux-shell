/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:57:50
 * @FilePath: /linux-shell/src/execute.c
 * @Descripttion: 命令执行模块
 */
#include "shell.h"

// 单命令执行（保持不变）
void execute_command(command_t* cmd) {
    if (cmd->args[0] == NULL) return;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        execvp(cmd->args[0], cmd->args);
        perror(cmd->args[0]);
        exit(127);
    }
    waitpid(pid, NULL, 0);
}


// ==========================================================
//       !!! 最终、完全重构的 execute_pipeline !!!
// ==========================================================
void execute_pipeline(command_t* cmds, int cmd_count) {
    int in_fd = STDIN_FILENO;
    pid_t pids[cmd_count];


    


    for (int i = 0; i < cmd_count; i++) {
        int pipe_fds[2];

        
        // 只有在不是最后一个命令时，才需要创建通向下一个命令的管道
        if (i < cmd_count - 1) {
            if (pipe(pipe_fds) < 0) {
                perror("pipe");
                return;
            }
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return;
        }

        if (pids[i] == 0) { // --- 子进程 ---
            // 1. 设置输入
            // 如果 in_fd 不是标准输入，说明它来自上一个管道，将本进程的输入重定向到它
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd); // 关闭旧的描述符
            }

            // 2. 设置输出
            // 如果不是最后一个命令，将本进程的输出重定向到新管道的写入端
            if (i < cmd_count - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                // 子进程完成重定向后，必须关闭它持有的新管道的两端
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }
            
            // 3. 执行命令
            execvp(cmds[i].args[0], cmds[i].args);
            // 如果 execvp 成功，下面的代码永远不会执行
            perror(cmds[i].args[0]);
            exit(EXIT_FAILURE);
        }

        // --- 父进程 ---
        // 父进程必须关闭它持有的、已经传给子进程的旧管道的读取端
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }
        // 如果不是最后一个命令，父进程必须关闭新管道的写入端，并保存读取端
        if (i < cmd_count - 1) {
            close(pipe_fds[1]);
            in_fd = pipe_fds[0];
        }
    }

    if (in_fd != STDIN_FILENO) {
        close(in_fd);  // ✅ 最后一轮管道读端也要关闭
    }

    // 等待所有子进程按顺序结束
    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}
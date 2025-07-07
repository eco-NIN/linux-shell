/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:57:50
 * @FilePath: /linux_shell/src/execute.c
 * @Descripttion: 命令执行模块
 */
#include "shell.h"

void execute_command(command_t* cmd) {
    // ... (检查空命令和内建命令的代码已在 main_loop 中) ...
    if (cmd->args[0] == NULL) {
        // 空命令
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    } else if (pid == 0) {
        // --- 子进程 ---
        int fd_in, fd_out;

        // 处理输入重定向
        if (cmd->input_file) {
            fd_in = open(cmd->input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO); // 将标准输入重定向到文件
            close(fd_in);
        }

        // 处理输出重定向
        if (cmd->output_file) {
            fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO); // 将标准输出重定向到文件
            close(fd_out);
        }
        
        // 执行命令
        if (execvp(cmd->args[0], cmd->args) == -1) {
            fprintf(stderr, "myshell: command not found: %s\n", cmd->args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        // --- 父进程 ---
        if (!cmd->is_background) {
            // 如果不是后台任务，则等待
            waitpid(pid, NULL, 0);
        } else {
            // 如果是后台任务，打印 PID 并且不等待
            printf("Started background process: [%d]\n", pid);
        }
    }
}

#include "shell.h"

void execute_pipeline(command_t* cmds, int cmd_count) {
    int pipe_fds[2];
    int in_fd = STDIN_FILENO; // 第一个命令的输入是标准输入
    pid_t pid;

    for (int i = 0; i < cmd_count; i++) {
        pipe(pipe_fds); // 创建管道

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // --- 子进程 ---

            // 将输入重定向到上一个命令的输出
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // 如果不是最后一个命令，将输出重定向到管道的写端
            if (i < cmd_count - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO);
            }

            // 关闭管道的读写两端
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            
            // 处理该命令自身的重定向
            // (此处代码省略，可以结合 execute_command 中的逻辑)

            // 执行命令
            if (handle_builtin_command(&cmds[i]) == 0) {
                 if (execvp(cmds[i].args[0], cmds[i].args) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_SUCCESS); // 内建命令执行完也退出子进程
        } else {
            // --- 父进程 ---
            close(pipe_fds[1]); // 父进程关闭写端
            if (in_fd != STDIN_FILENO) {
                close(in_fd); // 关闭上一轮的读端
            }
            in_fd = pipe_fds[0]; // 保存当前管道的读端，作为下一个命令的输入
        }
    }

    // 等待所有子进程结束
    for (int i = 0; i < cmd_count; i++) {
        wait(NULL);
    }
}
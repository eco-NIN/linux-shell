/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:57:50
 * @FilePath: /linux-shell/src/execute.c
 * @Descripttion: 命令执行模块-执行外部命令
 */
#include "shell.h"

/**
 * @description: 执行单个命令，支持I/O重定向和后台执行
 */
void execute_command(command_t* cmd) {
    if (cmd->args[0] == NULL) {
        return; // 空命令
    }

    pid_t pid = fork(); //第一步，克隆自己，创建子进程

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // --- 子进程 ---
        int fd_in, fd_out;

        // 处理输入重定向--结构体定义
        if (cmd->input_file) {
            // 打开一个文件，获取一个“文件描述符”（File Descriptor），这是一个代表该文件的整数。
            fd_in = open(cmd->input_file, O_RDONLY);
            if (fd_in == -1) {
                perror(cmd->input_file);
                exit(EXIT_FAILURE);
            }
            // dup2(old_fd, new_fd)是重定向的核心。它会复制一个文件描述符，让 new_fd 指向和 old_fd 同一个文件。
            // dup2(file_fd, STDOUT_FILENO) 就让“标准输出”（STDOUT_FILENO，通常是1）指向了我们用 open 打开的文件。
            dup2(fd_in, STDIN_FILENO); // 把标准输入指到这个文件
            close(fd_in);
        }

        // 处理输出重定向
        if (cmd->output_file) {
            fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror(cmd->output_file);
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO); // 把标准输出指向这个文件
            close(fd_out);
        }
        
        // 执行命令
        // 第二步：让子进程“变身”成外部命令
        execvp(cmd->args[0], cmd->args);
        // 如果 execvp 成功，下面的代码不会被执行
        // 如果 execvp 成功，子进程就已经是 ls 了，永远不会执行到这里
        perror(cmd->args[0]);
        exit(EXIT_FAILURE);
    } else {
        // --- 父进程 ---

        // 第三步：父进程等待子进程结束
        if (!cmd->is_background) {
            // 如果不是前台任务，则等待
            // 父进程用它来等待子进程结束。这是前后台执行的分水岭。
            waitpid(pid, NULL, 0);
        } else {
            // 如果是后台任务，打印 PID 并且不等待
            printf("[%d]\n", pid);
        }
    }
}


/**
 * @description: 执行一个包含多个命令的管道
 */

// 以 ls | grep .c 为例
// 解析: parser 将命令分割成两个 command_t 结构，一个给 ls，一个给 grep。
void execute_pipeline(command_t* cmds, int cmd_count) {
    int pipe_fds[2];
    int in_fd = STDIN_FILENO;
    pid_t pids[cmd_count];


    // 循环多次 
    for (int i = 0; i < cmd_count; i++) {
        if (i < cmd_count - 1) {
            // pipe(): 创建一个管道，返回两个文件描述符，一个用于读，一个用于写。
            // 创建管道: 父进程调用 pipe(pipe_fds)，得到 pipe_fds[0]（读取端）和 pipe_fds[1]（写入端）。
            if (pipe(pipe_fds) < 0) {
                perror("pipe");
                return;
            }
        }

        // fork(): 每次循环可以为管道中的每一个命令都创建一个子进程
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return;
        }

        if (pids[i] == 0) { // --- 子进程 ---


            // 如果这个命令不是第一个，那它就需要“把前一个命令的输出当作自己的输入”。
            // 而“前一个命令的输出”，在上一轮 pipe() 时保存在了 in_fd 中（读端）。
            if (in_fd != STDIN_FILENO) {
                // 同样用于“焊接水管”，将子进程的 stdin 或 stdout 连接到管道的相应端口。
                dup2(in_fd, STDIN_FILENO); // 把上一个命令的输出，接到当前命令的输入
                // close(): 在管道中极其重要。
                // 每个进程都必须关闭它自己用不到的管道端口，否则可能导致数据流“卡住”或死锁
                close(in_fd);
            }
            if (i < cmd_count - 1) {
                dup2(pipe_fds[1], STDOUT_FILENO); // 当前命令输出，接到管道写端
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }
            
            // 调用 execvp("ls", ...)
            // 最后调用 execvp("grep", ...)
            execvp(cmds[i].args[0], cmds[i].args);
            perror(cmds[i].args[0]);
            exit(EXIT_FAILURE);
        }

        // --- 父进程 ---
        if (in_fd != STDIN_FILENO) {
            close(in_fd);  //把当前的 in_fd 关掉（已经给了子进程）
        }
        if (i < cmd_count - 1) {
            close(pipe_fds[1]);
            in_fd = pipe_fds[0];  // ‼️把新的管道读端传下去，给下一个子命令用
        }
    }


    if (in_fd != STDIN_FILENO) {
        close(in_fd);  //  最后一轮管道读端也要关闭
    }

    // 等待所有子进程结束
    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}
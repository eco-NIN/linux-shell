/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:51:12
 * @FilePath: /linux-shell/src/parser.c
 * @Descripttion: 命令解析模块,这个模块负责将字符串拆分成 command_t 结构。
 */
#include "shell.h"

// 先声明一个辅助函数，用于清理 command_t 数组的内存
static void cleanup_cmds(command_t* cmds, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; cmds[i].args[j] != NULL; j++) {
            free(cmds[i].args[j]);
        }
    }
}

/**
 * @description: 统一的命令行解析函数
 * @param {char*} line - 完整的用户输入行
 * @param {command_t*} cmds - command_t 结构体数组
 * @return {int} - 解析出的命令数量
 */
int parse_line(char* line, command_t* cmds) {
    int cmd_count = 0;
    int argc = 0;
    char* buf = line;
    char* arg_start = NULL;
    int in_quote = 0;

    // 初始化第一个命令
    memset(&cmds[cmd_count], 0, sizeof(command_t));

    while (*buf != '\0') {
        // 跳过前导空白
        while (*buf == ' ' || *buf == '\t') buf++;
        if (*buf == '\0') break;

        // 检查管道符
        if (*buf == '|') {
            if (argc == 0) { // 管道符前没有命令
                fprintf(stderr, "myshell: syntax error near unexpected token `|'\n");
                cleanup_cmds(cmds, cmd_count);
                return 0;
            }
            cmds[cmd_count].args[argc] = NULL; // 结束当前命令的参数列表
            cmd_count++; // 移动到下一个命令
            argc = 0; // 重置参数计数器
            memset(&cmds[cmd_count], 0, sizeof(command_t)); // 初始化下一个命令
            buf++;
            continue;
        }
        
        // 解析一个参数
        arg_start = buf;
        if (*buf == '\"') {
            in_quote = 1;
            arg_start++; // 跳过开头的引号
            buf++;
            while (*buf != '\0' && *buf != '\"') buf++;
        } else {
            in_quote = 0;
            while (*buf != '\0' && *buf != ' ' && *buf != '\t' && *buf != '|') buf++;
        }
        
        char saved_char = *buf;
        *buf = '\0';
        cmds[cmd_count].args[argc++] = strdup(arg_start);
        *buf = saved_char;
        
        if (in_quote) {
            if (*buf == '\"') buf++; // 跳过结束的引号
            else {
                fprintf(stderr, "myshell: syntax error: unclosed quote\n");
                cleanup_cmds(cmds, cmd_count + 1);
                return 0;
            }
        }
    }

    if (argc > 0) {
        cmds[cmd_count].args[argc] = NULL;
        cmd_count++;
    }

    // 后处理：扫描每个命令的参数来查找重定向和后台符号
    for (int i = 0; i < cmd_count; i++) {
        for (int j = 0; cmds[i].args[j] != NULL; j++) {
            if (strcmp(cmds[i].args[j], "<") == 0) {
                cmds[i].input_file = strdup(cmds[i].args[j+1]);
                free(cmds[i].args[j]);
                free(cmds[i].args[j+1]);
                cmds[i].args[j] = NULL; // 从参数中移除
                cmds[i].args[j+1] = NULL;
            } else if (strcmp(cmds[i].args[j], ">") == 0) {
                cmds[i].output_file = strdup(cmds[i].args[j+1]);
                free(cmds[i].args[j]);
                free(cmds[i].args[j+1]);
                cmds[i].args[j] = NULL;
            } else if (strcmp(cmds[i].args[j], "&") == 0) {
                cmds[i].is_background = 1;
                free(cmds[i].args[j]);
                cmds[i].args[j] = NULL;
            }
        }
        // 清理参数数组中的 NULL “空洞”
        int write_idx = 0;
        for (int read_idx = 0; cmds[i].args[read_idx] != NULL; read_idx++) {
            if (cmds[i].args[read_idx] != NULL) { // 检查是多余的，但为了安全
                cmds[i].args[write_idx++] = cmds[i].args[read_idx];
            }
        }
        cmds[i].args[write_idx] = NULL;
    }

    return cmd_count;
}
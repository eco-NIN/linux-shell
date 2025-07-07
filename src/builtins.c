/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:58:06
 * @FilePath: /linux-shell/src/builtins.c
 * @Descripttion: 内建命令实现模块
 */
#include "shell.h"
#include <string.h> // 需用到 strcmp, strdup 等函数

// 内建命令的名称列表
const char* builtin_str[] = {
    "cd",
    "echo",
    "history",
    "type",
    "alias",
    "exit"
};

// 内建命令对应的函数指针数组
void (*builtin_func[])(char**) = {
    &builtin_cd,
    &builtin_echo,
    &builtin_history,
    &builtin_type,
    &builtin_alias,
    // exit 是特殊情况，直接在 handle 中处理
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}

// 总处理器，检查命令是否是内建命令并执行
int handle_builtin_command(command_t* cmd) {
    if (strcmp(cmd->args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(cmd->args[0], builtin_str[i]) == 0) {
            (*builtin_func[i])(cmd->args);
            return 1; // 找到了并执行了内建命令
        }
    }
    return 0; // 不是内建命令
}

// --- 具体实现 ---

void builtin_cd(char** args) {
    if (args[1] == NULL) {
        // 如果没有参数，则切换到 HOME 目录
        char* home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
        } else if (chdir(home) != 0) {
            perror("cd");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}

void builtin_echo(char** args) {
    int i = 1;
    while (args[i] != NULL) {
        // 处理环境变量
        if (args[i][0] == '$') {
            char* var = getenv(args[i] + 1);
            if (var) {
                printf("%s ", var);
            }
        } else {
            printf("%s ", args[i]);
        }
        i++;
    }
    printf("\n");
}


// // 其他内建命令先留空
// void builtin_history() { printf("history: not implemented yet\n"); }

// =================================================================
// == 功能一：History 实现部分
// =================================================================

// --- History 的全局变量 (用 static 限制在此文件内可见) ---
static char* history[HIST_SIZE];
static int history_count = 0;

/**
 * @description: 添加一条命令到历史记录的环形缓冲区
 * @param {const char*} cmd 用户输入的命令
 */
void add_to_history(const char* cmd) {
    // 忽略空命令或纯空格的命令
    if (cmd == NULL || strlen(cmd) == 0) return;

    // 如果历史记录中有相同的上一条命令，则不重复添加
    int last_index = (history_count > 0) ? (history_count - 1) % HIST_SIZE : -1;
    if (last_index != -1 && strcmp(history[last_index], cmd) == 0) {
        return;
    }
    
    // 如果环形缓冲区已满，需要先释放掉最旧的命令所占用的内存
    if (history_count >= HIST_SIZE && history[history_count % HIST_SIZE] != NULL) {
        free(history[history_count % HIST_SIZE]);
    }

    // 使用 strdup 复制命令字符串，为其分配新的内存
    history[history_count % HIST_SIZE] = strdup(cmd);
    history_count++;
}


// --- `history` 命令的具体实现 ---
void builtin_history(char** args) {
    int start_point = 0;
    // 如果记录超过了最大容量，只显示最近的 HIST_SIZE 条
    if (history_count > HIST_SIZE) {
        start_point = history_count - HIST_SIZE;
    }

    for (int i = start_point; i < history_count; i++) {
        // 通过取余运算正确地从环形缓冲区中获取索引
        printf("%5d  %s\n", i + 1, history[i % HIST_SIZE]);
    }
}



// void builtin_type() { printf("type: not implemented yet\n"); }
// void builtin_alias() { printf("alias: not implemented yet\n"); }
// src/builtins.c 中修正后的代码
void builtin_type(char** args) { printf("type: not implemented yet\n"); }
void builtin_alias(char** args) { printf("alias: not implemented yet\n"); }
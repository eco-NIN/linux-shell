/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:58:06
 * @FilePath: /linux-shell/src/builtins.c
 * @Descripttion: 内建命令实现模块
 */
#include "shell.h"
#include <string.h> // 需用到 strcmp, strdup 等函数
#include <sys/stat.h> // for `type` command
#include <unistd.h>   // for `access` in `type` command
#include <stdlib.h> // 确保包含了 stdlib.h for malloc


// =================================================================
// == 内建命令注册与分发
// =================================================================

// 内建命令的名称列表 (加入 unalias)
// 内建命令的名称列表
const char* builtin_str[] = {
    "cd",
    "echo",
    "history",
    "type",
    "alias",
    "unalias", // 新增 unalias 命令
    "exit"
};

// 内建命令对应的函数指针数组
void (*builtin_func[])(char**) = {
    &builtin_cd,
    &builtin_echo,
    &builtin_history,
    &builtin_type,
    &builtin_alias,
    &builtin_unalias, // 新增 unalias 命令
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
//void builtin_type(char** args) { printf("type: not implemented yet\n"); }
//void builtin_alias(char** args) { printf("alias: not implemented yet\n"); }

// =================================================================
// == 功能二：Alias 实现部分
// =================================================================

// --- Alias 的全局变量 ---
static Alias* alias_list_head = NULL; // 别名链表的头指针

/**
 * @description: 查找一个别名
 * @return {char*} 如果找到，返回对应的命令；否则返回 NULL
 */
static char* lookup_alias(const char* name) {
    for (Alias* current = alias_list_head; current != NULL; current = current->next) {
        if (strcmp(current->name, name) == 0) {
            return current->command;
        }
    }
    return NULL;
}


/**
 * @description: 设置或更新一个别名
 * @param {char*} name 别名
 * @param {char*} command 对应的命令
 */
static void set_alias(const char* name, const char* command) {
    // 检查是否已存在
    char* existing_command = lookup_alias(name);
    if (existing_command != NULL) {
        // 更新已存在的别名
        // `lookup_alias` 返回的指针指向 `command` 字段，但为了安全，我们还是遍历一遍
        for (Alias* current = alias_list_head; current != NULL; current = current->next) {
            if (strcmp(current->name, name) == 0) {
                free(current->command);
                current->command = strdup(command);
                return;
            }
        }
    }

    // 不存在，则创建新节点并插入到链表头部
    Alias* new_alias = (Alias*)malloc(sizeof(Alias));
    new_alias->name = strdup(name);
    new_alias->command = strdup(command);
    new_alias->next = alias_list_head;
    alias_list_head = new_alias;
}

/**
 * @description: 取消一个别名
 */
void builtin_unalias(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "unalias: usage: unalias name\n");
        return;
    }

    Alias* current = alias_list_head;
    Alias* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, args[1]) == 0) {
            if (prev == NULL) { // 要删除的是头节点
                alias_list_head = current->next;
            } else { // 删除中间或尾部节点
                prev->next = current->next;
            }
            free(current->name);
            free(current->command);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

/**
 * @description: `alias` 命令的具体实现
 */
// 在 src/builtins.c 中替换旧的 builtin_alias
void builtin_alias(char** args) {
    if (args[1] == NULL) {
        // 情况1: 只输入 `alias`，打印所有别名
        for (Alias* current = alias_list_head; current != NULL; current = current->next) {
            printf("alias %s='%s'\n", current->name, current->command);
        }
        return;
    }

    // --- 新增的修复逻辑：将所有参数重新拼接成一个字符串 ---
    char full_arg[MAX_CMD_LEN] = {0}; // 使用一个足够大的缓冲区
    strcpy(full_arg, args[1]);
    for (int i = 2; args[i] != NULL; i++) {
        strcat(full_arg, " "); // 用空格重新连接
        strcat(full_arg, args[i]);
    }
    // 此时，full_arg 的内容是 "ll='ls -alF'"，正是我们需要的！
    // -----------------------------------------------------

    char* eq_pos = strchr(full_arg, '=');

    if (eq_pos != NULL) {
        // 这是设置别名操作
        *eq_pos = '\0'; // 截断字符串，分离 name
        char* name = full_arg;
        char* command = eq_pos + 1;

        // 检查并去除可能包裹的单引号
        if (command[0] == '\'' && command[strlen(command) - 1] == '\'') {
            command[strlen(command) - 1] = '\0';
            command++;
        }
        set_alias(name, command);
    } else {
        // 这是查询单个别名操作，它只可能有一个参数
        char* existing_command = lookup_alias(args[1]);
        if (existing_command) {
            printf("alias %s='%s'\n", args[1], existing_command);
        } else {
            fprintf(stderr, "myshell: alias: %s: not found\n", args[1]);
        }
    }
}

/**
 * @description: 检查并展开别名。这是关键函数，会被 main_loop 调用。
 * @return {char*} 返回展开后的新命令字符串（需要调用者 free），如果不是别名则返回原命令的副本。
 */
// 在 src/builtins.c 中替换旧的 expand_alias
/**
 * @description: 检查并展开别名。这是关键函数，会被 main_loop 调用。
 * @return {char*} 返回展开后的新命令字符串（需要调用者 free），如果不是别名则返回原命令的副本。
 */
char* expand_alias(char* line) {
    // 如果行是空的，直接返回一个空的副本
    if (line == NULL || strlen(line) == 0) {
        return strdup("");
    }

    char first_word[MAX_CMD_LEN] = {0};
    char* rest_of_line = "";

    // 安全地复制第一个词
    char* first_space = strchr(line, ' ');
    
    if (first_space != NULL) {
        // 命令后面有其他参数
        int first_word_len = first_space - line;
        if (first_word_len < MAX_CMD_LEN) {
            strncpy(first_word, line, first_word_len);
            first_word[first_word_len] = '\0';
            rest_of_line = first_space + 1;
        } else {
            // 第一个词太长，异常情况处理
            return strdup(line);
        }
    } else {
        // 整行只有一个词
        strncpy(first_word, line, MAX_CMD_LEN - 1);
        first_word[MAX_CMD_LEN - 1] = '\0';
    }

    // 查找这个词是否是别名
    char* command = lookup_alias(first_word);

    if (command == NULL) {
        // 不是别名，返回原命令的副本
        return strdup(line);
    }

    // 是别名，需要拼接成新的命令字符串
    size_t new_size = strlen(command) + strlen(rest_of_line) + 2; // +2 for space and null terminator
    char* new_line = (char*)malloc(new_size);
    if (new_line == NULL) {
        perror("malloc");
        return strdup(line); // 内存分配失败，返回副本
    }

    if (strlen(rest_of_line) > 0) {
        snprintf(new_line, new_size, "%s %s", command, rest_of_line);
    } else {
        snprintf(new_line, new_size, "%s", command);
    }

    return new_line;
}



// =================================================================
// == `type` 命令实现
// =================================================================
void builtin_type(char** args) {
    if (args[1] == NULL) {
        return; // 参数不足
    }

    char* cmd_name = args[1];

    // 1. 检查是不是别名
    char* alias_cmd = lookup_alias(cmd_name);
    if (alias_cmd) {
        printf("%s is an alias for '%s'\n", cmd_name, alias_cmd);
        return;
    }

    // 2. 检查是不是内建命令 (需要在列表中加入 unalias)
    const char* local_builtin_str[] = {"cd", "echo", "history", "type", "alias", "exit", "unalias"};
    for (int i = 0; i < sizeof(local_builtin_str)/sizeof(char*); i++) {
        if (strcmp(cmd_name, local_builtin_str[i]) == 0) {
            printf("%s is a shell builtin\n", cmd_name);
            return;
        }
    }

    // 3. 检查是不是外部命令 (在 PATH 中查找)
    char* path_env = getenv("PATH");
    if (path_env == NULL) {
        fprintf(stderr, "type: %s: not found\n", cmd_name);
        return;
    }

    char* path_copy = strdup(path_env);
    char* dir = strtok(path_copy, ":");
    char full_path[1024];

    while(dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd_name);
        // 使用 access 检查文件是否存在且可执行
        if (access(full_path, X_OK) == 0) {
            printf("%s is %s\n", cmd_name, full_path);
            free(path_copy);
            return;
        }
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    fprintf(stderr, "type: %s: not found\n", cmd_name);
}
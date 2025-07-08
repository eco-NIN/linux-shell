/*
 * @Author: Yuzhe Guo
 * @Date: 2025-07-07 14:58:19
 * @FilePath: /linux-shell/src/completion.c
 * @Descripttion: 命令补全
 */

// 采用所有专业 Shell（包括 bash）都在使用的标准方案：GNU Readline 库。
// 1.实现命令补全，
// 2.Shell实现“行编辑”功能（比如用左右箭头移动光标、Ctrl+A 到行首等）。

// 安装 Readline 开发库，在系统终端中执行：
// Debian/Ubuntu: sudo apt-get install libreadline-dev
// CentOS/Fedora/RHEL: sudo yum install readline-devel
// macOS (使用 Homebrew): brew install readline (如果 make 时提示找不到，可能需要设置额外的 LDFLAGS 和 CPPFLAGS 指向 brew 的安装路径，例如 LDFLAGS="-L/opt/homebrew/opt/readline/lib" CPPFLAGS="-I/opt/homebrew/opt/readline/include")



// command_generator 是一个简化版，它只从一个固定的列表里查找命令。一个完整的实现需要去遍历 $PATH 环境变量里的所有目录来动态生成列表。但这个简化版足以让整个框架工作起来
#include "shell.h"
#include <readline/readline.h>
#include <dirent.h>

// 函数原型
static char* command_generator(const char* text, int state);
char** completion_callback(const char* text, int start, int end);

// 全局变量，用于在生成器中跟踪状态
static int list_index;
static char** command_list = NULL; // 用于存储所有匹配项

/**
 * @description: readline 的主回调函数，当用户按 Tab 时被调用
 */
char** completion_callback(const char* text, int start, int end) {
    // 关闭 readline 默认的文件名补全，我们自己来处理
    rl_attempted_completion_over = 1;
    
    // 如果是命令的第一个词（start=0），则进行命令补全
    if (start == 0) {
        // 利用readline 提供的命令补全函数“rl_completion_matches”，调用生成器函数，生成所有可能的匹配项
        // 显示: readline 接收到我们返回的匹配列表后，由它负责完成后续所有工作：如果只有一个匹配项，就自动补全；如果有多个，就显示列表给用户。
        return rl_completion_matches(text, command_generator);
    }
    
    // （计划扩展）否则，可以进行文件名或路径补全
    return NULL;
}

/**
 * @description: “生成器”函数，被 readline 重复调用以获取所有可能的匹配项
 */
char* command_generator(const char* text, int state) {
    // 如果是第一次调用 (state=0)，我们需要构建一个所有可能命令的列表
    if (state == 0) {
        list_index = 0;
        
        // 1. 获取所有内建命令和别名 (这里需要 builtins.c 提供函数)
        // 2. 获取 $PATH 中的所有可执行文件
        
        // 为了简化，我们先用一个静态列表来演示，之后再扩展
        const char* builtins[] = {"cd", "echo", "exit", "history", "alias", "unalias", "type", NULL};
        const char* externals[] = {"ls", "grep", "cat", "pwd", "make", NULL}; // 示例

        int text_len = strlen(text);
        int count = 0;
        
        // 释放上一次的列表
        if (command_list != NULL) {
            for(int i = 0; command_list[i] != NULL; i++) free(command_list[i]);
            free(command_list);
        }
        command_list = (char**)malloc(sizeof(char*) * 100); // 假设最多100个匹配

        // 匹配内建命令
        for (int i = 0; builtins[i] != NULL; i++) {
            if (strncmp(builtins[i], text, text_len) == 0) {
                command_list[count++] = strdup(builtins[i]);
            }
        }
        // 匹配外部命令
        for (int i = 0; externals[i] != NULL; i++) {
             if (strncmp(externals[i], text, text_len) == 0) {
                command_list[count++] = strdup(externals[i]);
            }
        }
        command_list[count] = NULL;
    }

    // 从列表中返回下一个匹配项
    if (command_list && command_list[list_index]) {
        return strdup(command_list[list_index++]);
    }

    // 没有更多匹配项
    // 释放列表内存
    if (command_list != NULL) {
        for(int i = 0; command_list[i] != NULL; i++) free(command_list[i]);
        free(command_list);
        command_list = NULL;
    }
    return NULL;
}
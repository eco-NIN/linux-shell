# linux-shell - 一个用 C 语言编写的 Linux Shell

这是一个在 Linux/macOS 环境下，使用 C 语言从零开始编写的命令解释程序（Shell）。项目旨在学习和实践操作系统的核心概念，包括进程创建与控制、文件描述符、管道、I/O 重定向以及系统调用。

# ✅ 已实现功能 (Implemented Features)

## 基本 Shell 循环 (Core Shell Loop)

  * 能显示一个命令提示符（Prompt），并且能动态显示当前工作目录。
  * 能读取用户输入的命令。
  * 能在一个循环中持续接收用户命令，直到用户输入 `exit` 或按下 `Ctrl+D`。

## 执行外部命令 (External Command Execution)

  * Shell 的核心功能。通过 `fork()` 和 `execvp()`，你的 Shell 可以执行系统中的任何外部命令，例如 `ls`, `pwd`, `cat`, `grep`, `vim`, `gcc` 等。
  * 父进程会使用 `waitpid()` 等待子进程执行完毕，然后再显示新的提示符。

## 内建命令 (Built-in Commands)

  * `cd [目录]`: 可以正确地改变 Shell 自身的工作目录。
  * `echo [内容]`: 可以打印文本，并且支持展开环境变量（如 `echo $HOME`）。
  * `exit`: 可以正常退出 Shell。
  * `history`: 可以显示用户输入的历史命令列表。
  * `alias [name='command']`: 可以创建、修改或显示命令别名。
  * `unalias <name>`: 可以删除一个已存在的别名。
  * `type <command>`: 可以准确判断一个命令是别名、内建命令，还是外部可执行文件（并显示其路径）。

## I/O 重定向与后台执行 (I/O Redirection & Background Execution)

  * **输出重定向**: `命令 > 文件` (例如 `ls -l > file.txt`) 的逻辑已经实现。
  * **输入重定向**: `命令 < 文件` (例如 `cat < file.txt`) 的逻辑也已实现。
  * **后台执行**: `命令 &` 可以让命令在后台运行，Shell 会立即返回提示符。

## 管道 (Pipes)

  * 能够解析由 `|` 连接的多个命令。
  * 通过 `pipe()` 和多个子进程，已经可以实现将前一个命令的标准输出连接到后一个命令的标准输入（例如 `ls | sort`）。

# 🟡 待实现的高级功能 (Advanced Features to be Implemented)

  * **命令补全 (Command Completion)**: 在用户输入命令时，按 `Tab` 键自动补全命令或文件名。这是现代 Shell 的标志性功能，也是我们下一个可以挑战的目标。
  * **作业控制 (Job Control)**: 实现 `jobs`, `fg`, `bg` 等命令，以更精细地管理后台进程。
  * **脚本执行 (Script Execution)**: 让 Shell 能够读取一个文件作为输入并逐行执行其中的命令。

-----

# 🚀 使用说明 (Usage)

## 编译 (Compilation)

本项目使用 `make`进行管理。请确保你的系统中已安装 `gcc` (或 `clang`) 和 `make` 工具。

1.  **编译项目**:
    在项目根目录下，直接运行 `make` 命令。

    ```bash
    make
    ```

    这将会编译 `src/` 目录下的所有源文件，并将生成的目标文件放在 `obj/` 目录下，最终在项目根目录生成一个名为 `myshell` 的可执行文件。

2.  **清理项目**:
    如果需要清理所有编译生成的文件，运行 `make clean`。

    ```bash
    make clean
    ```

    该命令会删除 `obj/` 目录和 `myshell` 可执行文件。

## 运行 (Running the Shell)

编译成功后，在项目根目录下运行 `myshell`。

```bash
./myshell
```

你会看到类似下面的命令提示符，表示你已经成功进入自己编写的 Shell：

```
/path/to/your/project/linux-shell$
```

要退出 Shell，可以直接输入 `exit` 命令并回车。

## 功能示例 (Feature Examples)

### 1\. 基本命令与管道

```bash
# 执行外部命令
ls -alF

# 使用管道将 ls 的输出传递给 grep 进行过滤
ls -l | grep ".c"
```

### 2\. I/O 重定向与后台执行

```bash
# 将 `ls -l` 的结果输出到文件 a.txt (覆盖写入)
ls -l > a.txt

# 查看 a.txt 的内容
cat a.txt

# 将 `echo` 的结果追加到 a.txt 文件末尾
echo "--- End of list ---" >> a.txt
cat a.txt

# 将 a.txt 作为 `cat` 命令的输入
cat < a.txt

# 在后台执行一个耗时5秒的命令，Shell 会立刻返回
sleep 5 &
```

### 3\. 内建命令

#### `cd` - 切换目录

```bash
# 进入 src 目录
cd src

# 查看当前路径 (外部命令)
pwd
```

#### `history` - 查看历史记录

```bash
# 显示最近输入的命令历史
history
```

#### `alias` / `unalias` - 设置和取消别名

```bash
# 设置一个别名 ll，使其等同于 `ls -l`
alias ll='ls -l'

# 直接使用别名 ll
ll

# 查看所有已设置的别名
alias

# 取消别名 ll
unalias ll

# 再次使用 ll，会提示命令未找到
ll
```

#### `type` - 查看命令类型

```bash
# 在设置别名 ll 之后
type ll
# > ll is an alias for 'ls -l'

# 查看内建命令
type cd
# > cd is a shell builtin

# 查看外部命令
type gcc
# > gcc is /usr/bin/gcc
```
# linux-shell

在 Linux/macOS 环境下，使用 C 语言从零开始编写的命令解释程序（Shell）。项目旨在学习和实践操作系统的核心概念，包括进程创建与控制、文件描述符、管道、I/O 重定向以及系统调用。

# 已实现功能 (Implemented Features)

## 基本 Shell 循环 (Core Shell Loop)

  * 能显示一个命令提示符（Prompt），并且能动态显示当前工作目录。
  * 能通过 `readline` 库读取用户输入的命令，并支持行编辑（如箭头移动）和历史记录。
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

## 命令补全 (基础版)

  * 集成了 GNU Readline 库，按 `Tab` 键可对命令进行补全。
  * 目前实现了一个基于预设列表的命令名补全，作为功能的演示。

-----

# 使用说明 (Usage)

## 编译 (Compilation)

本项目使用 `make`进行管理。请确保你的系统中已安装 `gcc` (或 `clang`)、`make` 以及 `readline` 开发库。

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

## 运行 (Running the Shell)

编译成功后，在项目根目录下运行 `myshell`。

```bash
./myshell
```

你会看到类似下面的命令提示符，表示你已经成功进入自己编写的 Shell：

```
/path/to/your/project/linux-shell$
```

要退出 Shell，可以直接输入 `exit` 命令或按 `Ctrl+D`。

## 功能示例 (Feature Examples)

### 1\. 基本命令与管道

```bash
# 执行外部命令
ls -alF

# 使用管道将 ls 的输出传递给 grep 进行过滤
ls -l src | grep ".c"
```

### 2\. I/O 重定向与后台执行

```bash
# 将 `ls -l` 的结果输出到文件 a.txt (覆盖写入)
ls -l > a.txt

# 查看 a.txt 的内容
cat a.txt

# 在后台执行一个耗时5秒的命令，Shell 会立刻返回
sleep 5 &
```

### 3\. 内建命令

```bash
# 切换目录
cd src

# 查看历史记录
history

# 设置、使用和取消别名
alias ll='ls -l'
ll
unalias ll

# 查看命令类型
type ls
type cd
```

### 4\. 命令补全 (基础版)

```bash
# 输入 l 然后按 Tab 键，会自动补全为 ls
l<Tab>

# 输入 c 然后连续按两次 Tab 键，会列出所有 c 开头的命令
c<Tab><Tab>
```

-----

# To-Do List (未来计划)

  - [ ] **完善命令补全功能**
      - [ ] 实现对 `$PATH` 环境变量中所有可执行文件的动态补全。
      - [ ] 增加文件名和目录路径补全功能。
      - [ ] 增加对别名（Alias）的补全支持。
  - [ ] **实现作业控制 (Job Control)**
      - [ ] 实现 `jobs` 命令来查看后台作业。
    <!-- end list -->
      * [ ] 实现 `fg` 和 `bg` 命令来控制作业的前后台切换。
      * [ ] 实现对 `Ctrl+Z` 信号的捕捉，以挂起当前正在运行的程序。
  - [ ] **支持脚本执行**
      - [ ] 让 Shell 能够接收一个文件名作为参数，并执行文件中的命令。
  - [ ] **高级功能**
      * [ ] 支持 `~` 符号的家目录展开。
      * [ ] 支持更复杂的命令提示符（Prompt）定制。
  - [ ] 实现history n 功能

# 补充：命令补全功能的实现
集成 Readline 实现命令补全

第一步：安装 Readline 开发库（如果之前没装过）

在系统终端中执行：

> Debian/Ubuntu: sudo apt-get install libreadline-dev

> CentOS/Fedora/RHEL: sudo yum install readline-devel

> macOS (使用 Homebrew): brew install readline (如果 make 时提示找不到，可能需要设置额外的 LDFLAGS 和 CPPFLAGS 指向 brew 的安装路径，例如 LDFLAGS="-L/opt/homebrew/opt/readline/lib" CPPFLAGS="-I/opt/homebrew/opt/readline/include")

  > history (应该会显示3条记录)
  
  > history 2 (应该只显示最近的2条：pwd 和 echo "hello")
  
  > history -c (清空所有历史)
  
  > history (这次应该什么都不显示)

  > !! 执行最近的一条历史命令
  
  > !n 执行第n条历史命令
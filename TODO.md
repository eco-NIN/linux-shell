<!--
 * @Author: Yuzhe Guo
 * @Date: 2025-07-08 18:18:37
 * @FilePath: /linux-shell/TODO.md
 * @Descripttion: 
-->
好的，这些是非常棒的想法！一个强大易用的历史系统是现代 Shell 的灵魂，而了解内建命令与外部命令的区别也是核心。

我们来逐一分析和实现。

-----

### **1. 关于 `chmod` 及其他类似命令**

首先，我来回答你关于 `chmod` 的问题。

`chmod` 是一个**外部命令**，和 `ls`、`grep`、`cat` 一样。它是一个存在于你系统路径中（通常是 `/bin/chmod`）的独立的可执行程序。

**好消息是：你的 Shell 已经可以完美地执行它了！** 你不需要为它编写任何新的 C 代码。你通过 `fork()` 和 `execvp()` 建立的命令执行机制，赋予了你的 Shell 调用整个 Linux/macOS 系统中成千上万个已有外部命令的能力。

你可以在你的 Shell 中直接测试：

```bash
# 创建一个测试文件
> touch testfile.txt

# 查看权限
ls -l testfile.txt
# > -rw-r--r--  ... testfile.txt

# 使用你的 Shell 执行 chmod
chmod 755 testfile.txt

# 再次查看权限，会发现已改变
ls -l testfile.txt
# > -rwxr-xr-x  ... testfile.txt
```

-----

### **2. 当前已实现的命令列表**

根据我们到目前为止的开发，你的 Shell 命令可以分为两大类：

#### **内建命令 (Built-in Commands)**

这些是我们用 C 语言亲自编写、作为 Shell 一部分的功能。它们不需要创建新的进程。

  * `cd`: 切换目录。
  * `echo`: 打印文本和环境变量。
  * `exit`: 退出 Shell。
  * `history`: 显示命令历史（我们马上来增强它）。
  * `alias`: 创建、修改或显示别名。
  * `unalias`: 取消别名。
  * `type`: 判断命令类型。

#### **可执行的外部命令 (External Commands)**

这是你的 Shell 最强大的地方。理论上，系统中**几乎所有**能在终端里运行的命令，你的 Shell 都能执行。

  * **文件操作**: `ls`, `cat`, `grep`, `cp`, `mv`, `rm`, `mkdir`, `chmod`, `chown` ...
  * **系统工具**: `pwd`, `ps`, `top`, `df`, `ping`, `ifconfig` ...
  * **开发工具**: `gcc`, `make`, `git`, `vim`, `docker` ...
  * **所有你通过 `apt`, `yum`, `brew` 安装的程序**。
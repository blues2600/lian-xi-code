/*
 *
 * 任务
 * 如何在某一特定时刻生成核心转储（core dump）文件，而同时进程得以继续执行？
 *
 * 思考
 * 1.生成核心转储的信号SIGABRT/SIGQUIT
 * 2.fork可以复刻一个进程
 * 3.父进程保持sleep，子进程给自己发信号生成转储并退出
 * 4.父进程继续执行
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error_functions.h"

// 参考信息 https://juejin.cn/s/ubuntu%2020.04%20core%20dump%20file%20location
// 本机转储文件目录 /var/lib/apport/coredump
// 查找核心转储文件 sudo find / -name "core.*" -exec ls -lh {} \;
// 暂时允许进程进行核心转储 ulimit -c unlimited 或者指定转储文件最大大小 ulimit -c 1024000字节


int main() {
    pid_t child = fork();

    switch (child) {
        case -1:
            errExit("fork");
        case 0:
            printf("i am child %lld\n",(long long)getpid());
            sleep(1000);  // 睡觉，等待信号
        default:
            sleep(3);
            kill(child, SIGQUIT);  // 向子进程发送信号
            if (wait(NULL) == -1)  // 发完信号开始等
                errExit("wait()");
            printf("现在父进程苏醒了\n");
    }
    return 0;
}

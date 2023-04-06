/*
 *
 * 错误处理：
 * 1.有errno，使用errMsg()，输出错误信息格式：errMsg("open ()");//ERROR [ENOENT
 * No such file or directory] open ()
 * 2.有errno，使用errExit()，程序退出，错误信息格式同上
 * 3.无errno，使用其他自定义错误输出
 *
 *
 * 任务
 * 请用 setitimer()实现 alarm()
 *
 *
 * 对alarm()的理解
 * 1.计时+生成SIGALRM 信号
 * 2. 参数为0则 alarm取消
 * 3.返回值：剩余秒数或0（到期）
 * 4.函数总是成功
 *
 * 步骤分解
 * 1.使用settimer创建定时器
 * 2.到了发送信号
 * 3.
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "error_functions.h"

// 自定义实现alarm函数

unsigned int MyAlarm(unsigned int seconds) {
    struct itimerval new;
    struct itimerval old, current;
    memset(&old, 0x0, sizeof(struct itimerval));
    memset(&new, 0x0, sizeof(struct itimerval));
    memset(&current, 0x0, sizeof(struct itimerval));

    new.it_value.tv_sec = seconds;

    if (setitimer(ITIMER_REAL, &new, &old))
        errExit("setitimer()");

    if (getitimer(ITIMER_REAL, &current))
        errExit("getitimer()");

    return (unsigned int)current.it_value.tv_sec;
}

// 信号处理器

static void handler(int sig) {
    if (sig == SIGALRM) {
        putchar('L');
        putchar('\n');
    }
}

int main(int argc, char* argv[]) {
    // 注册一个用户定义的信号处理器
    struct sigaction sigalrm;
    memset(&sigalrm, 0x0, sizeof(struct sigaction));
    sigalrm.sa_handler = &handler;
    sigalrm.sa_flags = SA_NODEFER;
    if (sigaction(SIGALRM, &sigalrm, NULL))
        errExit("sigaction()");

    MyAlarm(10);
    int i = 1000000;
    while (i) {
        i--;
        MyAlarm(0);  // 取消定时器
    }
    printf("hello\n");
    return 0;
}


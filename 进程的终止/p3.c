/*
 *
 * 使用 waitid()替换程序清单 26-3（child_status.c）中的 waitpid()。需要将对函数
 * print WaitStatus()的调用替换为打印 waitid()所返回 siginfo_t
 * 结构中相关字段的代码。
 *
 *
 * 1.替换waitpid为waitid
 * 2.修改printWaitStatus()
 */

// #define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309L

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <features.h>
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
#include <time.h>
#include <unistd.h>

#include "error_functions.h"

void printWaitStatus(siginfo_t *info) {
    if (info->si_code == CLD_EXITED)
        printf("子进程已通过调用_exit()而终止\n");

    if (info->si_code == CLD_KILLED)
        printf("子进程被某个信号所杀\n");

    if (info->si_code == CLD_STOPPED)
        printf("子进程因某个信号而停止\n");

    if (info->si_code == CLD_CONTINUED)
        printf("子进程因接收到（SIGCONT）信号而恢复执行\n");

    printf("进程id为%lld, 退出代码/信号值为%d\n", (long long)info->si_pid,
           info->si_status);
}

int main(int argc, char *argv[]) {
    pid_t childPid;
    siginfo_t sig_information;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [exit-status]\n", argv[0]);

    switch (fork()) {
        case -1:
            errExit("fork");

        case 0:
            printf("Child started with PID = %ld\n", (long)getpid());
            if (argc > 1)
                exit(EXIT_FAILURE);
            else
                _exit(EXIT_SUCCESS);

        default:
            for (;;) {
                // 这里的waitpid感兴趣的进程是哪些？它指定了哪些option？
                // wait任意子进程；
                // 子进程退出、子进程被信号暂停、子进程被信号恢复，都会被wait到
                sleep(2);
                childPid = waitid(P_ALL, 0, &sig_information, WEXITED);
                if (childPid == -1)
                    errExit("waitpid");

                printWaitStatus(&sig_information);
                exit(EXIT_SUCCESS);
            }
    }
}

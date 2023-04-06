/*
 *
 * 错误处理：
 * 1.有errno，使用errMsg()，输出错误信息格式：errMsg("open ()");//ERROR [ENOENT
 * No such file or directory] open ()
 * 2.有errno，使用errExit()，程序退出，错误信息格式同上
 * 3.无errno，使用其他自定义错误输出
 *
 *
 * 编写一个程序验证：如果调用 timer_create()时将参数 evp 置为 NULL，那么这就等
 * 同于将 evp 设为指向 sigevent 结构的指针，并将该结构中的 sigev_notify 置为
 * SIGEV_ SIGNAL，将 sigev_signo 置为 SIGALRM，将 si_value.sival_int 置为定时器
 * ID。
 */

#define _POSIX_C_SOURCE 199309L

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
#include <time.h>
#include <unistd.h>

#include "error_functions.h"

// 信号处理器
static void handler(int sig, siginfo_t *info, void *ucontext) {
    // 注意，printf函数在信号处理器中会有问题，这里只是为了练习
    if (sig == SIGALRM) {
        printf("进程收到SIGALRM信号就意味着信号是发送给进程的，且信号类型为SIGALRM\n");
        printf("si_signo=%d\n", info->si_signo);
        printf("si_code=%d\n", info->si_code);
        printf("si_status=%d\n", info->si_status);
        printf("union sigval si_value=%d\n", info->si_value.sival_int);
        printf("si_int=%d\n", info->si_int);
        printf("si_timerid=%d\n", info->si_timerid);
        printf("si_fd=%d\n", info->si_fd);
        printf("\nsi_overrun=%d\n", info->si_overrun);
        printf("\ntimer_getoverrun() return=%d\n", timer_getoverrun((timer_t) info->si_value.sival_ptr));
    }
}

int main(int argc, char *argv[]) {
    /* 注册一个用户定义的信号处理器,并且指定SA_SIGINFO 标志
     * 当指定这个标志时，信号处理程序地址通过 act.sa_sigaction 字段传递
     * 此时，信号处理器的参数有三个,如handler函数所示
     * */
    struct sigaction sigalrm;
    memset(&sigalrm, 0x0, sizeof(struct sigaction));
    sigalrm.sa_sigaction = &handler;
    sigalrm.sa_flags = SA_SIGINFO;
    if (sigaction(SIGALRM, &sigalrm, NULL))
        errExit("sigaction()");

    // 创建定时器，由于timer_create第二个参数而NULL
    // 定时器到期之后，发送SIGALRM信号给本进程
    timer_t timer;
    if (timer_create(CLOCK_REALTIME, NULL, &timer))
        errExit("timer_create()");
    printf("timer = %lld\n",(long long)timer);

    // 启动定时器
    struct itimerspec overtime;
    memset(&overtime, 0x0, sizeof(struct itimerspec));
    overtime.it_value.tv_sec = 3;     // 定时器到期时间
    overtime.it_interval.tv_sec = 2;  // 循环计时时间
    if (timer_settime(timer, 0, &overtime, NULL))
        errExit("timer_settime()");

    sleep(10);
    return 0;
}

/*
 *
 * 错误处理：
 * 1.有errno，使用errMsg()，输出错误信息格式：errMsg("open ()");//ERROR [ENOENT
 * No such file or directory] open ()
 * 2.有errno，使用errExit()，程序退出，错误信息格式同上
 * 3.无errno，使用其他自定义错误输出
 *
 *
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
#include <time.h>
#include <unistd.h>

#include "error_functions.h"

// 信号处理器
static void handler(int sig) {
    if (sig == SIGALRM) {
        putchar('L');
        putchar('\n');
    }
}

int main(int argc, char* argv[]) {
    time_t now = 0;
    char* time_string = NULL;
    struct tm* utc_time = NULL;
    struct tm* local_time = NULL;
    time_t return_time = 0;

    // 获得系统时间
    now = time(NULL);
    if (now == ((time_t)-1))
        errExit("time()");

    // 将时间转换为字符串
    time_string = ctime(&now);
    if (time_string == NULL)
        errExit("ctime()");
    printf("ctime() return string:%s", time_string);

    // 将time_t转换为tm结构，以格林威治时间（世界标准时间UTC）表示
    utc_time = gmtime(&now);
    if (utc_time == NULL)
        errExit("gmtime()");
    printf("UTC:%d/%d/%d %d:%d:%d\n", utc_time->tm_year + 1900,
           utc_time->tm_mon + 1, utc_time->tm_mday, utc_time->tm_hour,
           utc_time->tm_min, utc_time->tm_sec);

    // 受到TZ环境变量影响
    // 将time_t转换为tm结构，以用户时区表示
    local_time = localtime(&now);
    if (local_time == NULL)
        errExit("loclatime()");
    printf("Local:%d/%d/%d %d:%d:%d\n", utc_time->tm_year + 1900,
           utc_time->tm_mon + 1, utc_time->tm_mday, utc_time->tm_hour,
           utc_time->tm_min, utc_time->tm_sec);

    // 受到TZ环境变量影响
    // 将tm结构转换为time_t结构
    return_time = mktime(local_time);
    printf("the now = %lld,the return_time = %lld\n", (long long)now,
           (long long)return_time);

    // 注册一个用户定义的信号处理器
    struct sigaction sigabrt;
    memset(&sigabrt, 0x0, sizeof(struct sigaction));
    sigabrt.sa_handler = &handler;
    sigabrt.sa_flags = SA_NODEFER;
    if (sigaction(SIGALRM, &sigabrt, NULL))
        errExit("sigaction()");

    // 简单定时器，发出SIGALRM信号
    printf("定时器开始计时\n");
    alarm(5);

    int i = 1000000;
    while (i) {
        i--;
        alarm(0);  // 取消定时器
    }
    printf("hello\n");

    sleep(2);
    printf("i am wake up\n");

    // 返回CLOCK_REALTIME时钟的时间 clock_gettime
    struct timespec clock_time;
    memset(&clock_time, 0x0, sizeof(struct timespec));
    if (clock_gettime(CLOCK_REALTIME, &clock_time))
        errExit("clock_gettime()");

    time_string = ctime(&clock_time.tv_sec);
    if (time_string == NULL)
        errExit("ctime()");
    printf("clock_realtime:%s", time_string);

    // 返回CLOCK_MONOTONIC时钟的时间 clock_gettime
    memset(&clock_time, 0x0, sizeof(struct timespec));
    if (clock_gettime(CLOCK_MONOTONIC, &clock_time))
        errExit("clock_gettime()");
    printf("CLOCK_MONOTONIC时钟的时间为：%lld\n", (long long)clock_time.tv_sec);

    // 获得当前进程的时钟id  clock_getcpuclockid
    clockid_t clockid;
    if (clock_getcpuclockid(0, &clockid))
        errExit("clock_getcpuclockid()");

    // 获得当前进程时钟的值(进程耗费的CPU时间
    memset(&clock_time, 0x0, sizeof(struct timespec));
    if (clock_gettime(clockid, &clock_time))
        errExit("clock_gettime()");
    printf("当前进程cpu时间为：%lld\n", (long long)clock_time.tv_sec);

    // unix间隔定时器

    // 创建定时器，由于timer_create第而个参数而NULL
    // 定时器到期之后，发送SIGALRM信号给本进程
    timer_t test_timer;  // 定时器句柄
    if (timer_create(CLOCK_REALTIME, NULL, &test_timer))
        errExit("timer_create()");

    // 启动定时器
    struct itimerspec overtime;
    memset(&overtime, 0x0, sizeof(struct itimerspec));
    overtime.it_value.tv_sec = 3;  // 定时器到期时间
    overtime.it_interval.tv_sec =
        2;  // 循环定时，每次2秒(如果这里设置为1，则定时器可能会不起作用，因为信号延迟
    if (timer_settime(test_timer, 0, &overtime, NULL))
        errExit("timer_settime()");

    timer_delete(test_timer);
    

    return 0;
}

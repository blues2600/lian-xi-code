/*
 *
 *
 *  编写一程序，以 sigaction()来建立信号处理器函数，请验证 SA_RESETHAND 和
 *  SA_NODEFER 标志的效果
 *
 *
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 信号处理器
static void handler(int sig) {
    if (sig == SIGINT)  // ctrl+c信号
        printf("hello,new SIGINT\n");
}

/*
*
 * 没有设置SA_RESETHAND时，SIGINT信号会被信号处理器捕获，然后输出设定的目标字符串hello,new SIGINT
 *
 * 设置SA_RESETHAND时，第一次SIGINT信号会被信号处理器捕获，第二次会被内核执行默认行为
 *
 * SA_NODEFER覆盖SA_RESETHAND，信号会持续发送给信号处理器
 */

int main() {
    struct sigaction sigint;
    memset(&sigint, 0, sizeof(struct sigaction));

    sigint.sa_handler = &handler;
    sigint.sa_flags = SA_RESETHAND;
    sigint.sa_flags = SA_NODEFER;

    if (sigaction(SIGINT, &sigint, NULL)) {
        printf("sigaction() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1) {

    }
    return 0;
}



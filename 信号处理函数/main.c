/*
 *
 * 实现 abort()
 *
 *
 *
 * 对abort的理解
 * 作用：导致调用它的进程终止
 * 实现：发出SIGABRT信号终止进程。
 * 1.首先解除对信号SIGABRT的阻塞
 * 2.然后为调用进程发送该信号，就像raise()被调用一样
 * 3.如果SIGABRT信号进程的信号处理程序忽略或捕获，该abort()函数仍将终止进程。它通过恢复默认配置SIGABRT然后第二次提高信号来实现这一点。
 *
 *
 * 信号发送函数的选择
 * 1.kill可以向一个非常广泛的进程范围发送信号，但是本程序用不到
 * 2.raise非常非常适合用来实现本程序，因为它向调用进城本身发送信号
 * 3.此外，我们需要结束调用进程，所调用的信号类型raise也刚好满足
 */

/*
 *
 * 本程序参考了GNU libc abort函数源代码
 * 但实现略有不同
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

static int stage;  // 尝试myabort尝试次数

void MyAbort(void) {
    struct sigaction act;
    sigset_t mask;

    // 解除对SIGABRT信号的阻塞
    if (stage == 0) {
        stage++;
        if (sigemptyset(&mask)) {
            printf("sigemptyset() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (sigaddset(&mask, SIGABRT)) {
            printf("open() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (sigprocmask(SIG_UNBLOCK, &mask, NULL)) {
            printf("open() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // 向本进程发送SIGABRT信号，终止进程
        if (raise(SIGABRT)) {
            printf("raise() failed\n");
            exit(EXIT_FAILURE);
        }
    }

    // 当用户注册了信号处理器，并且flags = SA_RESETHAND
    // 当设置SA_RESETHAND时，对同一信号发送第二次就会被内核执行默认信号行为
    if (stage == 1) {
        stage++;
        if (raise(SIGABRT)) {
            printf("raise() failed\n");
            exit(EXIT_FAILURE);
        }
    }

    // 当用户注册了信号处理器，并且flags = SA_NODEFER
    // SA_NODEFER标志导致信号持续发送给信号处理器，不会让内核处理
    // 这里将会覆盖/移除用户定义的信号处理器
    if (stage == 2) {
        stage++;
        memset(&act, '\0', sizeof(struct sigaction));
        act.sa_handler = SIG_DFL;  // 信号处理器还原为默认
        if (sigfillset(&act.sa_mask)) {
            printf("sigemptyset() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        act.sa_flags = 0;

        // 注册新的信号处理器，覆盖/移除用户的信号处理器
        if (sigaction(SIGABRT, &act, NULL)) {
            printf("sigaction() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // 持续向进程发送信号，直到退出
    while (1) raise(SIGABRT);
}

// 信号处理器
static void handler(int sig) {
    if (sig == SIGABRT) {
        putchar('L');
        putchar('\n');
    }
}

int main() {
    // 注册一个用户定义的信号处理器
    struct sigaction sigabrt;
    memset(&sigabrt, 0x0, sizeof(struct sigaction));
    sigabrt.sa_handler = &handler;
    sigabrt.sa_flags = SA_NODEFER;
    if (sigaction(SIGABRT, &sigabrt, NULL)) {
        printf("sigaction() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    MyAbort();
    return getchar();
}

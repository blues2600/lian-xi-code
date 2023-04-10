/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* multi_SIGCHLD.c

   Demonstrate the use of a handler for the SIGCHLD signal, and that multiple
   SIGCHLD signals are not queued while the signal is blocked during the
   execution of the handler.
*/
#include <signal.h>
#include <sys/wait.h>
#include "print_wait_status.h"
#include "curr_time.h"
#include "tlpi_hdr.h"

static volatile int numLiveChildren = 0;
                /* Number of children started but not yet waited on */

static void
sigchldHandler(int sig)
{
    int status, savedErrno;
    pid_t childPid;

    /* UNSAFE: This handler uses non-async-signal-safe functions
       (printf(), printWaitStatus(), currTime(); see Section 21.1.2) */

    // 保存errno
    savedErrno = errno;         /* In case we modify 'errno' */

    printf("%s handler: Caught SIGCHLD\n", currTime("%T"));

    /* Do nonblocking waits until no more dead children are found */


    // wait任意子进程终止，status保存子进程状态信息
    // WNOHANG - 没有子进程终止则立即返回，不阻塞
    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("%s handler: Reaped child %ld - ", currTime("%T"),
                (long) childPid);
        printWaitStatus(NULL, status);
        numLiveChildren--;
    }

    // 如果调用进程并无与 pid 匹配的子进程，则 waitpid()报错-1，将错误号置为 ECHILD
    if (childPid == -1 && errno != ECHILD)
        errMsg("waitpid");

    sleep(5);           /* Artificially lengthen execution of handler */
    printf("%s handler: returning\n", currTime("%T"));

    errno = savedErrno;
}

int
main(int argc, char *argv[])
{
    int j, sigCnt;
    sigset_t blockMask, emptyMask;
    struct sigaction sa;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s child-sleep-time...\n", argv[0]);

    // 关闭标准输出的缓冲区
    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    sigCnt = 0;
    numLiveChildren = argc - 1;

    // 为SIGCHLD信号注册处理函数
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigchldHandler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    /* Block SIGCHLD to prevent its delivery if a child terminates
       before the parent commences the sigsuspend() loop below */

    // 将sigchld信号添加到掩码中，阻塞它
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);
    if (sigprocmask(SIG_SETMASK, &blockMask, NULL) == -1)
        errExit("sigprocmask");

    /* Create one child process for each command-line argument */

    for (j = 1; j < argc; j++) {
        switch (fork()) {
        case -1:
            errExit("fork");

        case 0:         /* Child - sleeps and then exits */
            sleep(getInt(argv[j], GN_NONNEG, "child-sleep-time"));
            printf("%s Child %d (PID=%ld) exiting\n", currTime("%T"),
                    j, (long) getpid());
            _exit(EXIT_SUCCESS);

        default:        /* Parent - loops to create next child */
            break;
        }
    }

    /* Parent comes here: wait for SIGCHLD until all children are dead */

    sigemptyset(&emptyMask);
    while (numLiveChildren > 0) {

        // 将进程的信号掩码置空，挂起进程，直到进程捕获到信号，并出信号处理函数返回
        // 当信号处理函数返回，sigsuspend将之前的掩码恢复
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");
        sigCnt++;       // 每收到一次CHLD信号+1
    }

    printf("%s All %d children have terminated; SIGCHLD was caught "
            "%d times\n", currTime("%T"), argc - 1, sigCnt);

    exit(EXIT_SUCCESS);
}

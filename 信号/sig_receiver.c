/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* sig_receiver.c

   Usage: sig_receiver [block-time]

   Catch and report statistics on signals sent by sig_sender.c.

   Note that although we use signal() to establish the signal handler in this
   program, the use of sigaction() is always preferable for this task.
*/
#define _GNU_SOURCE
#include <signal.h>
#include "signal_functions.h"           /* Declaration of printSigset() */
#include "tlpi_hdr.h"

// 计算每个信号的传递
static int sigCnt[NSIG];                /* Counts deliveries of each signal */
// 如果发送 SIGINT 则设置非零
static volatile sig_atomic_t gotSigint = 0;
                                        /* Set nonzero if SIGINT is delivered */

// 信号处理器
static void
handler(int sig)
{
    if (sig == SIGINT)  //对信号SIGINT的处理
        gotSigint = 1;
    else
        sigCnt[sig]++;  //非SIGINT信号传递给进程，sigCnt+1
}

int
main(int argc, char *argv[])
{
    int n, numSecs;
    sigset_t pendingMask, blockingMask, emptyMask;

    printf("%s: PID is %ld\n", argv[0], (long) getpid());

    /* Here we use the simpler signal() API to establish a signal handler,
       but for the reasons described in Section 22.7 of TLPI, sigaction()
       is the (strongly) preferred API for this task. */

    // 改变所有信号的默认处理方式,使用handler来处理所有信号
    for (n = 1; n < NSIG; n++)          /* Same handler for all signals */
        (void) signal(n, handler);      /* Ignore errors */

    /* If a sleep time was specified, temporarily block all signals,
       sleep (while another process sends us signals), and then
       display the mask of pending signals and unblock all signals */

    if (argc > 1) {
        numSecs = getInt(argv[1], GN_GT_0, NULL);

        sigfillset(&blockingMask);
        // 阻塞进程的所有信号，除了默认无法阻塞的之外
        if (sigprocmask(SIG_SETMASK, &blockingMask, NULL) == -1)
            errExit("sigprocmask");

        printf("%s: sleeping for %d seconds\n", argv[0], numSecs);
        sleep(numSecs);

        // 获得正在等待状态的信号集
        if (sigpending(&pendingMask) == -1)
            errExit("sigpending");

        printf("%s: pending signals are: \n", argv[0]);
        // 打印处于等待状态的信号集中的所有信号描述信息
        printSigset(stdout, "\t\t", &pendingMask);

        sigemptyset(&emptyMask);        /* Unblock all signals */
        // 取消所有信号的阻塞
        if (sigprocmask(SIG_SETMASK, &emptyMask, NULL) == -1)
            errExit("sigprocmask");
    }

    // 除非遇到SIGINT信号，否则停在这里
    while (!gotSigint)                  /* Loop until SIGINT caught */
        continue;

    // 打印每种信号在进程中的接收次数
    for (n = 1; n < NSIG; n++)          /* Display number of signals received */
        if (sigCnt[n] != 0)
            printf("%s: signal %d caught %d time%s\n", argv[0], n,
                    sigCnt[n], (sigCnt[n] == 1) ? "" : "s");

    exit(EXIT_SUCCESS);
}

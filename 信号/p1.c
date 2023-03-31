/*
 * 任务
 * 编写一程序，以 sigaction()来建立信号处理器函数，请验证 SA_RESETHAND 和
 * SA_NODEFER 标志的效果。
 */

static void
handler(int sig)
{
    if (sig == SIGINT)  //对信号SIGINT的处理
        gotSigint = 1;
    else
        sigCnt[sig]++;  //非SIGINT信号传递给进程，sigCnt+1
}


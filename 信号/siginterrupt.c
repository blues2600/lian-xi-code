/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* siginterrupt.c

   An implementation of the siginterrupt(3) library function.
*/
#include <stdio.h>
#include <signal.h>

int
siginterrupt(int sig, int flag)
{
    int status;
    struct sigaction act;  //定义信号如何处理

    // 这里不定义信号的新行为，只是保存信号的默认行为
    status = sigaction(sig, NULL, &act);
    if (status == -1)
        return -1;

    if (flag)
        act.sa_flags &= ~SA_RESTART; //flags = flags - RESTART  系统调用不要重启
    else
        act.sa_flags |= SA_RESTART;  //flags = flags + RESTART  重启系统调用

    return sigaction(sig, &act, NULL); //注册信号的新行为
}

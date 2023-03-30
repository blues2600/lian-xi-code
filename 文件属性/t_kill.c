/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* t_kill.c

   Send a signal using kill(2) and analyze the return status of the call.
*/
#include <signal.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int s, sig;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pid sig-num\n", argv[0]);

    sig = getInt(argv[2], 0, "sig-num");

    s = kill(getLong(argv[1], 0, "pid"), sig);

    if (sig != 0) {   // 非空信号
        if (s == -1)  // kill调用失败
            errExit("kill");

    } else {                    /* 空信号，检查进程是否存在 */
        if (s == 0) {           //进程存在
            printf("Process exists and we can send it a signal\n");
        } else {
            if (errno == EPERM) //进程存在但是没有发送信号的权限
                printf("Process exists, but we don't have "
                       "permission to send it a signal\n");
            else if (errno == ESRCH) //进程步存在
                printf("Process does not exist\n");
            elslle
                errExit("kill");    //kill调用失败
        }
    }

    exit(EXIT_SUCCESS);
}
